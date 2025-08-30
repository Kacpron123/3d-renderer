#pragma once

#include "Vec.hpp"
#include "Mesh.h"
#include "TGAImage.h"
#include <vector>
#include <memory>
#include <map>
class Shader;
class Scene {
   private:
   mat<4,4> modelview;
   mat<4,4> projection;
   mat<4,4> viewport;
   std::vector<std::vector<double>> zbuffer; 
   std::map<std::string,std::shared_ptr<Mesh>> Meshes;
   struct Light{
      vec3 pos;
      // TGAColor color;
      float intensity;
   };
   std::vector<Light> lights;
   /// @brief Rasterizes a line into the given image.
   /// @param a The first endpoint of the line in clip space.
   /// @param b The second endpoint of the line in clip space.
   /// @param image The image to rasterize the line into.
   /// @param color The color to use for the line.
   void raster_line(vec4 a, vec4 b, TGAImage &image, TGAColor color);
   
   /// @brief Rasterizes a triangle into the given image.
   /// @param clip_verts The three vertices of the triangle in clip space.
   /// @param image The image to rasterize the triangle into.
   /// @param color The color to use for the triangle.
   void rasterize(const vec4 clip_verts[3], TGAImage &image, TGAColor color);
   void rasterize_shader(int iface,const vec4 clip_verts[3], TGAImage &image,Shader s);
   public:
   Scene();
   bool draw_zbuffer=false;
   bool drawAxis=true;
   enum Format{WIREFRAME, SOLID,RENDER} format=WIREFRAME;
   void addMesh(std::shared_ptr<Mesh> mesh);
   void setCamera(const vec3 eye, const vec3 center, const vec3 up);
   /// @brief Sets the projection matrix based on the given parameters.
   /// @param fovy Field of view in the y direction, in degrees.
   /// @param aspect Aspect ratio that determines the field of view in the x direction.
   /// @param zNear The distance to the near clipping plane.
   /// @param zFar The distance to the far clipping plane.
   void setProjection(float fovy, float aspect, float zNear, float zFar);
   /// @brief orthografic projection
   void setProjection(float f);
   void addLight(vec3 pos,float intensity);
   std::shared_ptr<Mesh> getObject(std::string name){return Meshes[name];}
   void setViewport(int x,int y,int width,int height);
   void draw(TGAImage& image);
   ~Scene() = default;
};