#pragma once
#include <vector>
#include <map>
#include <string>
#include "Object3D.h"
#include "Vec.hpp"
#include "Material.h"
#include <memory>
class TGAImage;

class Mesh : public Object3D{
   std::vector<vec3> verts;
   std::vector<vec3> norms;
   std::vector<vec2> uvs;
   // faces
   std::vector<vec3i> faces_verts;
   std::vector<vec3i> faces_norms;
   std::vector<vec3i> faces_uvs;
   /// @brief materials
   /// it is quaranteed if any mtl is specified than there is default with name None
   std::map<std::string,std::shared_ptr<Material>> materials;
   mat4 modelMatrix=once<4>();
public:
   Mesh() = default;
   Mesh(const std::string &filename);
   Mesh(const Mesh &mesh) = default;
   int nverts() const { return verts.size(); }
   int nfaces() const { return faces_verts.size();}
   
   vec3 vert(int i) const { return verts[i]; }
   vec3 vert(int iface, int nvert) const{ return verts[faces_verts[iface][nvert]]; }
   vec3i ivert(int iface) const{return faces_verts[iface];}
   vec3 normal(int i) const { return norms[i]; }
   vec3 normal(int iface, int nvert) const{ return norms[faces_norms[iface][nvert]]; }
   
   const std::vector<vec3>& get_verts() const { return verts; }
   vec2 uv(int iface,int nvert) const { return uvs[faces_uvs[iface][nvert]];}
   
   const mat4& getModelMatrix() const {return modelMatrix;};
   void rotateX(float angle_degrees);
   void rotateY(float angle_degrees);
   void rotateZ(float angle_degrees);
   void move(vec3 move);
   void scale(vec3 scale);
   void scale(double f){scale({f,f,f});}
   auto getMaterials() const {return materials;};

   friend void render_cuda(TGAImage&, const mat4&, const mat4&, const mat4&, const std::map<std::string, std::shared_ptr<Mesh>>&);
    
};