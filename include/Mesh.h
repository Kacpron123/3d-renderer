#pragma once
#include "Vec.hpp"
#include <vector>
#include "Object3D.h"
#include <string>
class TGAImage;

class Mesh : public Object3D{
   std::vector<vec3> verts;
   std::vector<vec3> norms;
   std::vector<vec2> uvs;
   // faces
   std::vector<vec3i> faces_verts;
   std::vector<vec3i> faces_norms;
   std::vector<vec2i> faces_uvs;

   mat4 modelMatrix=once<4>();
public:
   Mesh() = default;
   Mesh(const std::string &filename);
   Mesh(const Mesh &mesh) = default;
   int nverts() const { return verts.size(); }
   int nfaces() const { return faces_verts.size(); }
   
   vec3 vert(int i) const { return verts[i]; }
   vec3 vert(int iface, int nvert) const{ return verts[faces_verts[iface][nvert]]; }
   vec3i ivert(int iface) const{return faces_verts[iface];}
   vec3 normal(int i) const { return norms[i]; }
   vec3 normal(int iface, int nvert) const{ return norms[faces_norms[iface][nvert]]; }
   
   std::vector<vec3> get_verts() const { return verts; }
   
   const mat4& getModelMatrix() const {return modelMatrix;};
   void rotateX(float angle_degrees);
   void rotateY(float angle_degrees);
   void rotateZ(float angle_degrees);
   void move(vec3 move);
   void scale(vec3 scale);
   void scale(double f){scale({f,f,f});}
};