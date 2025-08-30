#include "Tracer.cuh"
#include "TGAImage.h"
#include "Vec.hpp"
#include "Mesh.h"
#include "cuda_runtime.h"
#include <iostream>
#include <vector>

#define CHECK_CUDA(call) do { \
    cudaError_t err = call; \
    if (err != cudaSuccess) { \
        std::cerr << "CUDA Error: " << cudaGetErrorString(err) << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
    } \
} while(0)

// Helper function to calculate a ray-triangle intersection using the Möller–Trumbore algorithm.
__device__ bool intersect_triangle(const vec3& ray_origin, const vec3& ray_dir, const vec3& v0, const vec3& v1, const vec3& v2,float& t, float& u, float& v) {
   const float EPSILON = 1e-5f;
   vec3 edge1 = v1 - v0;
   vec3 edge2 = v2 - v0;
   vec3 h = cross(ray_dir, edge2);
   float a = dot(edge1, h);
   if (a > -EPSILON && a < EPSILON)
      return false;
   float f = 1.0f / a;
   vec3 s = ray_origin - v0;
   u = f * dot(s, h);
   if (u < 0.0f || u > 1.0f)
      return false;
   vec3 q = cross(s, edge1);
   v = f * dot(ray_dir, q);
   if (v < 0.0f || u + v > 1.0f)
      return false;
   t = f * dot(edge2, q);
   if (t > EPSILON) {
      return true;
   }
   return false;
}
// for debugging if needed
__device__ void printf_mat4(const char* name,const mat4& m) {
   printf("%s:\n", name);
   for (int i = 0; i < 4; ++i) {
      printf("[%f, %f, %f, %f]\n", m[i][0], m[i][1], m[i][2], m[i][3]);
   }
}

// The CUDA kernel. This function runs on the GPU.
// Each thread handles one pixel.
__global__ void renderKernel(unsigned char* d_image_data, int width, int height, const DeviceMesh* d_meshes,int num_meshes,const Light* d_lights,int num_lights,const mat4* d_modelview,const mat4* d_projection) {
   int x = blockIdx.x * blockDim.x + threadIdx.x;
   int y = blockIdx.y * blockDim.y + threadIdx.y;

   if (x >= width || y >= height) {
      return;
   }
   // if(x!=400 || y!=300) return;

   mat4 modelview=*d_modelview;
   mat4 projection=*d_projection;
   mat4 inv_modelview = modelview.invert();
   mat4 inv_projection = projection.invert();
   
   // Convert pixel coordinates to NDC space [-1, 1]
   vec4 ray_dir_ndc = {
      (2.0f * (x + 0.5f) / width - 1.0f),
      (2.0f * (y + 0.5f) / height - 1.0f),
      -1.0f, 1.0f
   };

   // Transform ray direction from screen space to world space
   vec4 eye_h=inv_projection*ray_dir_ndc;
   vec3 ray_dir_world = normalized(convert_to_size<3>(inv_modelview* vec4{eye_h.x,eye_h.y,-1.0f,0.0f}));
   vec3 ray_origin_world = convert_to_size<3>(inv_modelview * vec4{0, 0, 0, 1}); //tranpose becuase mat4 is row-major
  
   Ray ray_world = {ray_origin_world, ray_dir_world};
   int closest_mesh=-1,closest_face=-1;
   float t,u,v;
   float closest_t = 1e10f; // Initialize with a large value
   
   // Iterate through all meshes in the scene
   for (int i = 0; i < num_meshes; ++i) {
      const DeviceMesh& mesh = d_meshes[i];
      // Transform the ray into the mesh's local space
      mat4 inv_modelMatrix = mesh.modelMatrix.invert();
      vec3 ray_origin_local = convert_to_size<3>(inv_modelMatrix * convert_to_size<4>(ray_world.origin, 1.0));
      vec3 ray_dir_local = normalized(convert_to_size<3>(inv_modelMatrix * convert_to_size<4>(ray_world.direction, 0.0)));
      // Iterate through all faces of the mesh
      for (int j = 0; j < mesh.num_faces; ++j) {
         vec3i face = mesh.d_faces_verts[j];
         vec3 v0 = mesh.d_verts[face[0]];
         vec3 v1 = mesh.d_verts[face[1]];
         vec3 v2 = mesh.d_verts[face[2]];
         float temp_u,temp_v;
         if (intersect_triangle(ray_origin_local, ray_dir_local, v0, v1, v2, t, temp_u,temp_v)) {
            if (t < closest_t && t > 0.0f) {
               closest_t = t;
               closest_mesh=i;
               closest_face=j;
               u=temp_u;
               v=temp_v;
         }}}}
   if(closest_face==-1){
      d_image_data[(y * width + x) * 3 + 0] = 0;
      d_image_data[(y * width + x) * 3 + 1] = 0;
      d_image_data[(y * width + x) * 3 + 2] = 0;
      return;
   }

   const DeviceMesh& mesh=d_meshes[closest_mesh];
   mat4 inv_modelMatrix_face=mesh.modelMatrix.invert();
   // Sample diffuse texture
   vec3 tex_color;
   if (mesh.d_diffuse_map_data.d_data != nullptr) {
      vec3i face_uv = mesh.d_faces_uvs[closest_face];
      vec2 uv0 = mesh.d_uvs[face_uv[0]];
      vec2 uv1 = mesh.d_uvs[face_uv[1]];
      vec2 uv2 = mesh.d_uvs[face_uv[2]];
      
      vec2 uv = {
         (1.0f-u-v) * uv0.x + u * uv1.x + v * uv2.x,
         (1.0f-u-v) * uv0.y + u * uv1.y + v * uv2.y
      };
      
      int tex_x = (int)(uv.x * (mesh.d_diffuse_map_data.texture_width -1) +0.5f);
      int tex_y = (int)(uv.y * (mesh.d_diffuse_map_data.texture_height -1) +0.5f);

      
      // Clamp coordinates
      tex_x = max(0, min(tex_x, mesh.d_diffuse_map_data.texture_width - 1));
      tex_y = max(0, min(tex_y, mesh.d_diffuse_map_data.texture_height - 1));
      
      int tex_idx = (tex_y * mesh.d_diffuse_map_data.texture_width + tex_x) * 3;
      tex_color.x = mesh.d_diffuse_map_data.d_data[tex_idx + 0];
      tex_color.y = mesh.d_diffuse_map_data.d_data[tex_idx + 1];
      tex_color.z = mesh.d_diffuse_map_data.d_data[tex_idx + 2];
   } else {
      tex_color = vec3{0,0,0};
   }
   
   
   vec3 point_world=ray_origin_world+closest_t*ray_dir_world;
   vec3 final_color_vec = {0.0f, 0.0f, 0.0f};
   
   // Get interpolated normal
   vec3i face_n = mesh.d_faces_norms[closest_face];
   vec3 n0 = mesh.d_norms[face_n[0]];
   vec3 n1 = mesh.d_norms[face_n[1]];
   vec3 n2 = mesh.d_norms[face_n[2]]; 
   vec3 interpolated_normal = normalized(n0 * u + n1 * v + n2 * (1.0f - u - v));
   
      // caluclate light
      for(int li=0;li<num_lights;li++){
         vec3 light_pos_world=d_lights[li].pos;
         float intensity=d_lights[li].intensity;
         
         // shadow rays:
         bool in_shadow=false;
         for (int i = 0; !in_shadow && i < num_meshes; ++i) {
         const DeviceMesh& mesh = d_meshes[i];
         // Transform the ray into the mesh's local space
         mat4 inv_modelMatrix = mesh.modelMatrix.invert();
         vec3 shadow_ray_origin_local = convert_to_size<3>(inv_modelMatrix * convert_to_size<4>(point_world, 1.0));
         vec3 light_pos_local = convert_to_size<3>(inv_modelMatrix * convert_to_size<4>(light_pos_world, 1.0));
         vec3 shadow_ray_dir_local = normalized(light_pos_local - shadow_ray_origin_local);
         float t_shadow;
         float light_dist=norm(light_pos_world-point_world);
         // Iterate through all faces of the mesh
         for (int j = 0; !in_shadow && j < mesh.num_faces; ++j) {
            vec3i face = mesh.d_faces_verts[j];
            vec3 v0 = mesh.d_verts[face[0]];
            vec3 v1 = mesh.d_verts[face[1]];
            vec3 v2 = mesh.d_verts[face[2]];
            float temp_u,temp_v;
            if (intersect_triangle(shadow_ray_origin_local, shadow_ray_dir_local, v0, v1, v2, t_shadow, temp_u,temp_v)) {
               // if(t_shadow > 1e-4f && t_shadow < light_dist)
               in_shadow=true;
            }}}

         if(!in_shadow){
            vec3 light_dir_local = normalized(convert_to_size<3>(inv_modelMatrix_face * convert_to_size<4>(light_pos_world - point_world, 0.0)));
            float diffuse = fmaxf(dot(interpolated_normal, light_dir_local), 0.0f);
            final_color_vec = final_color_vec + tex_color * diffuse * intensity;
         }
      }
      
   // final_color_vec = tex_color; // color of texture
   // repair bgr to rgb
   float temp=final_color_vec.x;
   final_color_vec.x=final_color_vec.z;
   final_color_vec.z=temp;

   vec3 final_color=final_color_vec;

   
   
   // Convert final color to TGAColor format and write to image buffer
   d_image_data[(y * width + x) * 3 + 0] = static_cast<unsigned char>(fminf(final_color.x, 255.0f));
   d_image_data[(y * width + x) * 3 + 1] = static_cast<unsigned char>(fminf(final_color.y, 255.0f));
   d_image_data[(y * width + x) * 3 + 2] = static_cast<unsigned char>(fminf(final_color.z, 255.0f));
}

// Global function to be called from the host (CPU) code
// This is the entry point for CUDA rendering.
void render_cuda(TGAImage& image, const Scene& scene) {
   // Prepare data on the host (CPU) ---
   int width = image.width();
   int height = image.height();
   std::vector<unsigned char> h_image_data(width * height * 3); // Host-side image data
  
   // Create a vector of DeviceMesh structs on the host
   std::vector<DeviceMesh> h_device_meshes(scene.Meshes.size());
   // Pointers for device memory
   DeviceMesh* d_meshes;
   Light* d_lights;
   unsigned char* d_image_data;
   mat4 *d_modelview;
   mat4 *d_projection;

   // Copy data to the device (GPU) ---
   // Allocate device memory for the image
   CHECK_CUDA(cudaMalloc(&d_image_data, h_image_data.size()));
   // Allocate device memory for the meshes
   CHECK_CUDA(cudaMalloc(&d_meshes, scene.Meshes.size() * sizeof(DeviceMesh)));
   // lights
   CHECK_CUDA(cudaMalloc(&d_lights, scene.lights.size() * sizeof(Light)));
   CHECK_CUDA(cudaMemcpy(d_lights,scene.lights.data(),scene.lights.size()*sizeof(Light),cudaMemcpyHostToDevice));
   // copy matricies
   CHECK_CUDA(cudaMalloc(&d_modelview,sizeof(mat4))); 
   CHECK_CUDA(cudaMalloc(&d_projection,sizeof(mat4))); 
   CHECK_CUDA(cudaMemcpy(d_modelview,&scene.modelview,sizeof(mat4),cudaMemcpyHostToDevice ));
   CHECK_CUDA(cudaMemcpy(d_projection,&scene.projection,sizeof(mat4),cudaMemcpyHostToDevice ));
   int mesh_idx = 0;
   for (const auto& pair : scene.Meshes) {
      const auto& mesh = pair.second;
      DeviceMesh& dmesh=h_device_meshes[mesh_idx];
      // Mesh matrix
      // CHECK_CUDA(cudaMalloc(&h_))
      // Copy vertex data
      CHECK_CUDA(cudaMalloc(&dmesh.d_verts, mesh->nverts() * sizeof(vec3)));
      CHECK_CUDA(cudaMemcpy(dmesh.d_verts, mesh->verts.data(), mesh->nverts() * sizeof(vec3), cudaMemcpyHostToDevice));
      // Copy face data
      CHECK_CUDA(cudaMalloc(&dmesh.d_faces_verts, mesh->nfaces() * sizeof(vec3i)));
      CHECK_CUDA(cudaMemcpy(dmesh.d_faces_verts, mesh->faces_verts.data(), mesh->nfaces() * sizeof(vec3i), cudaMemcpyHostToDevice));
      // Copy UV data
      CHECK_CUDA(cudaMalloc(&dmesh.d_uvs, mesh->uvs.size() * sizeof(vec2)));
      CHECK_CUDA(cudaMemcpy(dmesh.d_uvs, mesh->uvs.data(), mesh->uvs.size() * sizeof(vec2), cudaMemcpyHostToDevice));
      // Copy UV face
      CHECK_CUDA(cudaMalloc(&dmesh.d_faces_uvs, mesh->faces_uvs.size() * sizeof(vec3i)));
      CHECK_CUDA(cudaMemcpy(dmesh.d_faces_uvs, mesh->faces_uvs.data(), mesh->faces_uvs.size() * sizeof(vec3i), cudaMemcpyHostToDevice));
      // Copy normal data
      CHECK_CUDA(cudaMalloc(&dmesh.d_norms, mesh->norms.size() * sizeof(vec3)));
      CHECK_CUDA(cudaMemcpy(dmesh.d_norms, mesh->norms.data(), mesh->norms.size() * sizeof(vec3), cudaMemcpyHostToDevice));
      // Copy normal face
      CHECK_CUDA(cudaMalloc(&dmesh.d_faces_norms, mesh->faces_norms.size() * sizeof(vec3i)));
      CHECK_CUDA(cudaMemcpy(dmesh.d_faces_norms, mesh->faces_norms.data(), mesh->faces_norms.size() * sizeof(vec3i), cudaMemcpyHostToDevice));

      // Copy texture data to device
      for(auto mattemp: mesh->materials){
         auto material=mattemp.second;
         TGAImage default_diffuse = material->diffuse_map;
         CHECK_CUDA(cudaMalloc(&h_device_meshes[mesh_idx].d_diffuse_map_data.d_data, default_diffuse.width() * default_diffuse.height() * 3));
         CHECK_CUDA(cudaMemcpy(h_device_meshes[mesh_idx].d_diffuse_map_data.d_data, default_diffuse.get(), default_diffuse.width() * default_diffuse.height() * 3, cudaMemcpyHostToDevice));
         h_device_meshes[mesh_idx].d_diffuse_map_data.texture_width = default_diffuse.width();
         h_device_meshes[mesh_idx].d_diffuse_map_data.texture_height = default_diffuse.height();
         break;
      }
   

      // Set other properties
      h_device_meshes[mesh_idx].num_verts = mesh->nverts();
      h_device_meshes[mesh_idx].num_faces = mesh->nfaces();
      h_device_meshes[mesh_idx].num_uvs = mesh->uvs.size();
      h_device_meshes[mesh_idx].num_norms = mesh->norms.size();
      h_device_meshes[mesh_idx].modelMatrix = mesh->getModelMatrix();

      mesh_idx++;
    }

   // Copy the array of DeviceMesh structs to the device
   CHECK_CUDA(cudaMemcpy(d_meshes, h_device_meshes.data(), scene.Meshes.size() * sizeof(DeviceMesh), cudaMemcpyHostToDevice));

   // --- Step 3: Launch the kernel ---
   dim3 blockSize(16, 16);
   dim3 gridSize((width + blockSize.x - 1) / blockSize.x, (height + blockSize.y - 1) / blockSize.y);
   renderKernel<<<gridSize, blockSize>>>(d_image_data, width, height, d_meshes, scene.Meshes.size(),d_lights,scene.lights.size(), d_modelview, d_projection);
   CHECK_CUDA(cudaGetLastError()); // Check for kernel launch errors
   CHECK_CUDA(cudaDeviceSynchronize()); // Wait for the GPU to finish

   
   // --- Step 4: Copy the result back to the host ---
   CHECK_CUDA(cudaMemcpy(h_image_data.data(), d_image_data, h_image_data.size(), cudaMemcpyDeviceToHost));
    
   // --- Step 5: Clean up and save image ---
   // Free device memory
   CHECK_CUDA(cudaFree(d_image_data));
   for (long unsigned int i = 0; i < h_device_meshes.size(); i++) {
      CHECK_CUDA(cudaFree(h_device_meshes[i].d_verts));
      CHECK_CUDA(cudaFree(h_device_meshes[i].d_faces_verts));
      CHECK_CUDA(cudaFree(h_device_meshes[i].d_uvs));
      CHECK_CUDA(cudaFree(h_device_meshes[i].d_faces_uvs));
      CHECK_CUDA(cudaFree(h_device_meshes[i].d_norms));
      CHECK_CUDA(cudaFree(h_device_meshes[i].d_faces_norms));
      if (h_device_meshes[i].d_diffuse_map_data.d_data != nullptr) {
         CHECK_CUDA(cudaFree(h_device_meshes[i].d_diffuse_map_data.d_data));
      }
   }
   CHECK_CUDA(cudaFree(d_lights));
   CHECK_CUDA(cudaFree(d_meshes));
   CHECK_CUDA(cudaFree(d_projection));
   CHECK_CUDA(cudaFree(d_modelview));

   // Convert the raw data to a TGA image
   for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
         int idx = (y * width + x) * 3;
         TGAColor color(h_image_data[idx], h_image_data[idx+1], h_image_data[idx+2]);
         image.set(x, y, color);
      }
   }
}

