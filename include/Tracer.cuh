#pragma once

#include "Vec.hpp"
#include "Mesh.h"
#include "cuda_runtime.h"
#include <map>
#include <memory>
#include <iostream>

struct Ray{
   vec3 origin;
   vec3 direction;
};

struct Texture{
   unsigned char* d_data;

   int texture_width;
   int texture_height;
};

struct DeviceMesh {
   vec3* d_verts;
   vec3i* d_faces_verts;
   vec2* d_uvs;
   vec3i* d_faces_uvs;
   vec3* d_norms;
   vec3i* d_faces_norms;
   mat4 modelMatrix;
   std::size_t num_verts;
   std::size_t num_faces;
   std::size_t num_uvs;
   std::size_t num_norms;

   // textures
   Texture d_diffuse_map_data;
   Texture d_ambient_map_data;
   Texture d_specular_color_map_data;
   Texture d_specular_highlight_map_data;
   Texture d_bump_map_data;
   Texture d_dissolve_map_data;
   Texture d_displacement_map_data;
   Texture d_emission_map_data;
};

/// A helper function to calculate a ray-triangle intersection.
/// \param ray_origin The origin of the ray
/// \param ray_dir The direction of the ray
/// \param v0 The first vertex of the triangle
/// \param v1 The second vertex of the triangle
/// \param v2 The third vertex of the triangle
/// \param t The distance of the intersection (out)
/// \param u The u-coordinate of the intersection (out)
/// \param v The v-coordinate of the intersection (out)
/// \returns true if the ray intersects the triangle, false otherwise
__device__ bool intersect_triangle(const vec3& ray_origin, const vec3& ray_dir, const vec3& v0, const vec3& v1, const vec3& v2,float& t, float& u, float& v);

/// This is the entry point for CUDA rendering.
/// \param image The output image to render to.
/// \param modelview The modelview matrix to use.
/// \param projection The projection matrix to use.
/// \param viewport The viewport matrix to use.
/// \param meshes A map of mesh names to meshes to render.
/// \throws cudaError_t if an error occurs.
void render_cuda(TGAImage& image, const mat4& modelview, const mat4& projection, const mat4& viewport,const std::map<std::string,std::shared_ptr<Mesh>>& meshes);
