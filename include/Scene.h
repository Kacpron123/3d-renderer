#pragma once

#include "Vec.hpp"
#include "Mesh.h"
#include "TGAImage.h"
#include <vector>
class Scene {
   private:
   mat<4,4> modelview;
   mat<4,4> projection;
   mat<4,4> viewport;
   public:
   bool drawAxis=true;
   enum Format{WIREFRAME, SOLID,RENDER} format=WIREFRAME;
   std::vector<Mesh> Meshes;
   void line3D(const vec3,const vec3,TGAImage&,const TGAColor) const;
   void rasterize(const vec4 clip_verts[3], TGAImage &image);
   Scene();
   void addMesh(Mesh &mesh){Meshes.push_back(mesh);}
   void setCamera(const vec3 eye, const vec3 center, const vec3 up);
   /// @brief Sets the projection matrix based on the given parameters.
   /// @param fovy Field of view in the y direction, in degrees.
   /// @param aspect Aspect ratio that determines the field of view in the x direction.
   /// @param zNear The distance to the near clipping plane.
   /// @param zFar The distance to the far clipping plane.
   void setProjection(float fovy, float aspect, float zNear, float zFar);
   /// @brief orthografic projection
   void setProjection(float f);
   Mesh& getObject(int i){return Meshes[i];}
   void setViewport(int x,int y,int width,int height);
   void draw(TGAImage& image) const;
   ~Scene() = default;
};