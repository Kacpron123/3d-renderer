#include "Scene.h"
#include "TGAImage.h"
#include "Vec.hpp"
#include "Graphic.h"
// rand()
#include <algorithm>
#include <cstdlib>
#include <ctime>

#include <iostream>
// be careful, {X,Y,Z} is actually {right,up,left}


Scene::Scene(){
   setCamera({3,3,3},{0,0,0},{0,1,0});;
   // setProjection(norm(vec3{1,1,3}));
   setProjection(50.0f,800./600.,0.1,100.0);
   setViewport(0,0,800,600);
}

void Scene::setCamera(const vec3 eye, const vec3 center, const vec3 up){
   vec3 z = normalized(eye - center);
   vec3 x = normalized(cross(up,z));
   vec3 y = cross(z,x);
   mat4 A = mat4{convert_to_size<4>(x),convert_to_size<4>(y),convert_to_size<4>(z),vec4{0,0,0,1}};
   mat4 B = once<4>();
   B[0][3] += -eye.x; B[1][3] += -eye.y; B[2][3] += -eye.z; 
   modelview = A*B;
}
void Scene::setProjection(float f){
   projection = {vec4{1,0,0,0},vec4{0,-1,0,0},vec4{0,0,1,0},vec4{0,0,-1/f,0}};
}
void Scene::setProjection(float fovy_degrees, float aspect, float zNear, float zFar) {
    // Convert field of view from degrees to radians, as tan() expects radians.
    float fovy_radians = fovy_degrees * M_PI / 180.0f;
    float tanHalfFovy = tan(fovy_radians / 2.0f);

    float m00 = 1.0f / (aspect * tanHalfFovy);
    float m11 = 1.0f / tanHalfFovy;
    float m22 = -(zFar + zNear) / (zFar - zNear);
    float m23 = -2.0f * (zFar * zNear) / (zFar - zNear);
    float m32 = -1.0f;
    float m33 = 0.0f;

    projection = {
        vec4{m00, 0, 0, 0},   
        vec4{0, m11, 0, 0},   
        vec4{0, 0, m22, m23}, 
        vec4{0, 0, m32, m33}  
    };

}
void Scene::setViewport(int x,int y,int width,int height){
   viewport = {
      vec4{width/2.,0,0,static_cast<double>(x+width/2)},
      vec4{0,height/2.,0,static_cast<double>(y+height/2)},
      vec4{0,0,1,0},
      vec4{0,0,0,1}};
   zbuffer=std::vector<std::vector<double>>(height, std::vector<double>(width, 0));
   }
   

void Scene::raster_line(vec4 p0_clip, vec4 p1_clip, TGAImage &image, TGAColor color) {
   if (p0_clip.w == 0.0f || std::fabs(p0_clip.w) < std::numeric_limits<float>::epsilon() ||
      p1_clip.w == 0.0f || std::fabs(p1_clip.w) < std::numeric_limits<float>::epsilon()) {
      return; 
   }
   vec4 p0_ndc = p0_clip / p0_clip.w;
   vec4 p1_ndc = p1_clip / p1_clip.w;

   vec3 p0_screen = convert_to_size<3>(viewport * p0_ndc);
   vec3 p1_screen = convert_to_size<3>(viewport * p1_ndc);

   bool steep = false;
   if (std::abs(static_cast<int>(p0_screen.x) - static_cast<int>(p1_screen.x)) < std::abs(static_cast<int>(p0_screen.y) - static_cast<int>(p1_screen.y))) {
      std::swap(p0_screen.x, p0_screen.y);
      std::swap(p1_screen.x, p1_screen.y);
      steep = true;
   }
   if (p0_screen.x > p1_screen.x) {
      std::swap(p0_screen, p1_screen);
   }

   int x0 = static_cast<int>(p0_screen.x);
   int y0 = static_cast<int>(p0_screen.y);
   int x1 = static_cast<int>(p1_screen.x);
   int y1 = static_cast<int>(p1_screen.y);

   double z0 = p0_screen.z; // NDC Z from viewport transform
   double z1 = p1_screen.z; // NDC Z from viewport transform

   int dx = x1 - x0;
   int dy = y1 - y0;

   double derror2 = std::abs(dy) * 2;
   double error2 = 0;
   int y = y0;

   for (int x = x0; x <= x1; x++) {
      double t = (dx == 0) ? 0.0 : (static_cast<double>(x - x0) / dx);
      double current_z = z0 * (1.0 - t) + z1 * t;

      int current_x = steep ? y : x;
      int current_y = steep ? x : y;

      if (current_x >= 0 && current_x < image.width() && current_y >= 0 && current_y < image.height()) {
         if (current_z < zbuffer[current_y][current_x]) {
            zbuffer[current_y][current_x] = current_z;
            image.set(current_x, current_y, color);
         }
      }

      error2 += derror2;
      if (error2 > dx) {
         y += (y1 > y0 ? 1 : -1);
         error2 -= dx * 2;
      }
   }
}
class Shader{
   //temporary
   const Mesh &mesh;
   TGAImage texture;
   public:
   Shader();
   Shader(const Mesh &m):mesh(m){
      texture.read_tga_file("../obj/Crystal/None_BaseColor.tga");
   };
TGAColor pixel(const vec3& bc_clip_corrected, int iface) {
   //texture mapping
   // Retrieve UVs for the current face from the mesh
   vec2 uv0 = mesh.uv(iface, 0);
   vec2 uv1 = mesh.uv(iface, 1);
   vec2 uv2 = mesh.uv(iface, 2);

   // Perform linear interpolation using the perspective-corrected barycentric coordinates
   vec2 interpolated_uv;
   interpolated_uv.x = bc_clip_corrected.x * uv0.x + bc_clip_corrected.y * uv1.x + bc_clip_corrected.z * uv2.x;
   interpolated_uv.y = bc_clip_corrected.x * uv0.y + bc_clip_corrected.y * uv1.y + bc_clip_corrected.z * uv2.y;

   // Sample the texture
   TGAColor color = texture.get(static_cast<int>(interpolated_uv.x * texture.width()),
                                 static_cast<int>(interpolated_uv.y * texture.height()));

   // Ensure clamping is done in get() or manually here:
   int tex_x = static_cast<int>(interpolated_uv.x * texture.width());
   int tex_y = static_cast<int>(interpolated_uv.y * texture.height());
   tex_x = std::max(0, std::min(tex_x, texture.width() - 1));
   tex_y = std::max(0, std::min(tex_y, texture.height() - 1));
   color = texture.get(tex_x, tex_y);

   return color;
}
};

void Scene::draw(TGAImage& image){
   srand(time(NULL));
   for(auto &line:zbuffer) for(double &v:line) v=std::numeric_limits<double>::max(); //clearing zbuffer

   if(drawAxis){
      double length = 2.0;
      TGAColor colors[] = {red, green, blue};  
      for (int i = 0; i < 3; i++) {
         vec4 v = {0, 0, 0, 1};
         v[i] = length;
         v = modelview * v;
         v = projection * v;
         vec4 A={0,0,0,1};
         A = modelview * A;
         A = projection * A;
         raster_line(A, v, image, colors[i]);
      }
   }

   // to speed up calculations, calculate matrix at the start
   mat4 matrix=once<4>();
   for(const auto &mesh : Meshes){
      std::vector<vec4> draw_verts;
      matrix=projection* modelview*mesh.getModelMatrix();
      for(const vec3 &vert : mesh.get_verts()){
         // object space
         vec4 v=convert_to_size<4>(vert,1.);

         v=matrix*v;
         // apply object matrix
         // v=mesh.getModelMatrix()*v;

         // here how points are converted:
         // world space > view space
         // v=modelview*v;
         // view space > clip space
         // v=projection*v;
         // // NDC(Normalized Device Coordinates) > viewport   ]
         // v=v/v[3];                                          ]moved to methods
         // // screen space                                    ]raster_line and rasterization
         // v=viewport*v;                                      ]
         // // perpective division
         // //here v.z is depth and v.w=1.0
         draw_verts.push_back(v);
      }
      
      Shader shader(mesh);
      // drawing:
      switch (format)
      {
      case WIREFRAME:
         for( int i=0;i<mesh.nfaces();i++){
            const vec3i d=mesh.ivert(i);
            raster_line(draw_verts[d[0]],draw_verts[d[1]],image,TGAColor(255,255,255));
            raster_line(draw_verts[d[1]],draw_verts[d[2]],image,TGAColor(255,255,255));
            raster_line(draw_verts[d[2]],draw_verts[d[0]],image,TGAColor(255,255,255));
         }
         break;
      case SOLID:
         for(int i=0;i<mesh.nfaces();i++){
            const vec3i d=mesh.ivert(i);
            vec3 normal=face_normal(convert_to_size<3>(draw_verts[d[0]]),convert_to_size<3>(draw_verts[d[1]]),convert_to_size<3>(draw_verts[d[2]]));
            int r = static_cast<int>((normal.x + 1.0) / 2.0 * 255.0);
            int g = static_cast<int>((normal.y + 1.0) / 2.0 * 255.0);
            int b = static_cast<int>((normal.z + 1.0) / 2.0 * 255.0);
            TGAColor color(r, g, b);
            vec4 clip_verts[3] = { draw_verts[d[0]], draw_verts[d[1]], draw_verts[d[2]] };
            rasterize(clip_verts, image, color);
         }
         break;
      case RENDER:
         for(int i=0;i<mesh.nfaces();i++){
            const vec3i d=mesh.ivert(i);
            vec3 normal=face_normal(convert_to_size<3>(draw_verts[d[0]]),convert_to_size<3>(draw_verts[d[1]]),convert_to_size<3>(draw_verts[d[2]]));
            int r = static_cast<int>((normal.x + 1.0) / 2.0 * 255.0);
            int g = static_cast<int>((normal.y + 1.0) / 2.0 * 255.0);
            int b = static_cast<int>((normal.z + 1.0) / 2.0 * 255.0);
            TGAColor color(r, g, b);
            vec4 clip_verts[3] = { draw_verts[d[0]], draw_verts[d[1]], draw_verts[d[2]] };
            rasterize_shader(i,clip_verts, image, shader);
         }
         break;
      }
   }
   
   if(draw_zbuffer){ // saving z buffer
   TGAImage buffer(zbuffer[0].size(),zbuffer.size(),3);
   double zbuffer_max = [*this]() {
      double max_value = -std::numeric_limits<double>::max();
      for (const auto& row : zbuffer) {
         for (double z : row) {
            if (z != std::numeric_limits<double>::max() && z > max_value) {
               max_value = z;
            }
         }
      }
      return max_value;
   }();
   double zbuffer_min = [*this]() {
      double min_value = std::numeric_limits<double>::max();
      for (const auto& row : zbuffer) {
         for (double z : row) {
            if (z != std::numeric_limits<double>::max() && z < min_value) {
               min_value = z;
            }
         }
      }
      return min_value;
   }();
   for (int y = 0; y < buffer.height(); ++y) {
       for (int x = 0; x < buffer.width(); ++x) {
         double z_value = zbuffer[y][x];
         if(zbuffer[y][x]==std::numeric_limits<double>::max()) continue;
         std::uint8_t intensity = static_cast<std::uint8_t>(255 * ((z_value-zbuffer_min)/(zbuffer_max-zbuffer_min)));
         buffer.set(x, y, TGAColor(intensity, intensity, intensity));
      }}
      buffer.write_tga_file("zbuffer.tga");
   }

}
void Scene::rasterize(const vec4 clip_verts[3], TGAImage &image, TGAColor color) {
   vec4 ndc_verts[3];
   for (int i :{0,1,2}) {
      if (clip_verts[i].w == 0.0f || std::fabs(clip_verts[i].w) < std::numeric_limits<float>::epsilon()) {
         return;
      }
      ndc_verts[i] = clip_verts[i] / clip_verts[i].w;
   }

   vec4 screen_verts[3] = {
      viewport * ndc_verts[0],
      viewport * ndc_verts[1],
      viewport * ndc_verts[2]
   };

   vec2 pts2[3] = {
      convert_to_size<2>(screen_verts[0]),
      convert_to_size<2>(screen_verts[1]),
      convert_to_size<2>(screen_verts[2])
   };

   int bbminx = std::max(0, static_cast<int>(std::min(std::min(pts2[0].x, pts2[1].x), pts2[2].x)));
   int bbminy = std::max(0, static_cast<int>(std::min(std::min(pts2[0].y, pts2[1].y), pts2[2].y)));
   int bbmaxx = std::min(image.width() - 1, static_cast<int>(std::max(std::max(pts2[0].x, pts2[1].x), pts2[2].x)));
   int bbmaxy = std::min(image.height() - 1, static_cast<int>(std::max(std::max(pts2[0].y, pts2[1].y), pts2[2].y)));

   #pragma omp parallel for
   for (int x = bbminx; x <= bbmaxx; x++) {
      for (int y = bbminy; y <= bbmaxy; y++) {
         vec3 bc_screen = barycentric(pts2, {static_cast<double>(x), static_cast<double>(y)});

         vec3 bc_clip = { ndc_verts[0].w * bc_screen.x, ndc_verts[1].w * bc_screen.y, ndc_verts[2].w * bc_screen.z };
         bc_clip = bc_clip / (bc_clip.x + bc_clip.y + bc_clip.z);

         double frag_depth = bc_clip * vec3{ ndc_verts[0].z, ndc_verts[1].z, ndc_verts[2].z };
         if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0 || frag_depth > zbuffer[y][x]) continue;

         zbuffer[y][x] = frag_depth;
         image.set(x, y, color);
      }
   }
}

void Scene::rasterize_shader(int iface,const vec4 clip_verts[3], TGAImage &image,Shader shader){
   vec4 ndc_verts[3];
   for (int i :{0,1,2}) {
      if (clip_verts[i].w == 0.0f || std::fabs(clip_verts[i].w) < std::numeric_limits<float>::epsilon()) {
         return;
      }
      ndc_verts[i] = clip_verts[i] / clip_verts[i].w;
   }

   vec4 screen_verts[3] = {
      viewport * ndc_verts[0],
      viewport * ndc_verts[1],
      viewport * ndc_verts[2]
   };

   vec2 pts2[3] = {
      convert_to_size<2>(screen_verts[0]),
      convert_to_size<2>(screen_verts[1]),
      convert_to_size<2>(screen_verts[2])
   };

   int bbminx = std::max(0, static_cast<int>(std::min(std::min(pts2[0].x, pts2[1].x), pts2[2].x)));
   int bbminy = std::max(0, static_cast<int>(std::min(std::min(pts2[0].y, pts2[1].y), pts2[2].y)));
   int bbmaxx = std::min(image.width() - 1, static_cast<int>(std::max(std::max(pts2[0].x, pts2[1].x), pts2[2].x)));
   int bbmaxy = std::min(image.height() - 1, static_cast<int>(std::max(std::max(pts2[0].y, pts2[1].y), pts2[2].y)));
   

   #pragma omp parallel for
   for (int x = bbminx; x <= bbmaxx; x++) {
      for (int y = bbminy; y <= bbmaxy; y++) {
         vec3 bc_screen = barycentric(pts2, {static_cast<double>(x), static_cast<double>(y)});

         vec3 bc_clip = { ndc_verts[0].w * bc_screen.x, ndc_verts[1].w * bc_screen.y, ndc_verts[2].w * bc_screen.z };
         bc_clip = bc_clip / (bc_clip.x + bc_clip.y + bc_clip.z);

         double frag_depth = bc_clip * vec3{ ndc_verts[0].z, ndc_verts[1].z, ndc_verts[2].z };
         if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0 || frag_depth > zbuffer[y][x]) continue;

         TGAColor color = shader.pixel(bc_clip,iface);
         zbuffer[y][x] = frag_depth;
         image.set(x, y, color);
      }
   }
}