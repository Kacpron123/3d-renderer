#include "Mesh.h"
#include <fstream>
#include <sstream>
Mesh::Mesh(const std::string &filename){
   std::ifstream in(filename);
   std::string line;
   if (!in.is_open()) {
      std::cerr << "can't open file " << filename << "\n";
      return;
   }
   while (std::getline(in, line)) {
      std::istringstream iss(line);
      std::string token;
      iss >> token;
      // TODO use map<string,lambda> to save Mesh
      if(token == "#") continue; // comment
      else if(token == "mtllib"){
         auto new_materials=Material::read_mtl(filename.substr(0, filename.find_last_of('.')) + ".mtl");
         for(const auto& mat:new_materials){
            materials[mat.first] = mat.second;
         }
      }
      else if(token == "s"){
         // TODO implement
      }
      else if (token == "v") { // vertex
         vec3 v;
         iss >> v.x >> v.y >> v.z;
         verts.push_back(v);
      }
      else if(token == "vt"){
         vec2 uv;
         iss >> uv.x >> uv.y;
         uv.y=uv.y; // filping y becuase obj files are inverted horizontally
         uvs.push_back(uv);
      }
      else if(token == "vn"){
         vec3 n;
         for(int i:{0,1,2})
            iss >> n[i];
         norms.push_back(normalized(n));
      }
      else if (token == "f") {
      int count=0;
      vec3i v, vt, vn;
      while (iss >> token) {
         if(count==3){
            std::cerr<<"Implementation only for triangles\n";
            return;
         }
         size_t pos = token.find('/');
         if (pos != std::string::npos) {
            v[count] = std::stoi(token.substr(0, pos))-1;
            std::string second_place=token.substr(pos + 1, token.find('/', pos + 1) - pos - 1);
            vt[count] = second_place=="" ? 0 : std::stoi(second_place)-1;
            vn[count] = std::stoi(token.substr(token.find('/', pos + 1) + 1))-1; 
         }
         count++;
         
      }
      if(count<3){
         std::cerr<<"corrupted file, abort\n";
         return;
      }
      faces_verts.push_back(v);
      faces_uvs.push_back(vt);
      faces_norms.push_back(vn);
      }
   }
}

void Mesh::rotateX(float angle_degrees){
   float angle_radians=angle_degrees * M_PI / 180.0;
   float c=std::cos(angle_radians);
   float s=std::sin(angle_radians);
   mat4 rotation_matrix=once<4>();
   rotation_matrix[1][1]=c; rotation_matrix[2][1]=s;
   rotation_matrix[1][2]=-s; rotation_matrix[2][2]=c;
   modelMatrix=modelMatrix*rotation_matrix;
}
void Mesh::rotateY(float angle_degrees){
   float angle_radians = angle_degrees * M_PI / 180.0;
   float c = std::cos(angle_radians);
   float s = std::sin(angle_radians);
   mat4 rotation_matrix = mat4::identity();
   rotation_matrix[0][0] = c; rotation_matrix[2][0] = -s;
   rotation_matrix[0][2] = s; rotation_matrix[2][2] = c;
   modelMatrix = modelMatrix * rotation_matrix;
}
void Mesh::rotateZ(float angle_degrees){
   float angle_radians = angle_degrees * M_PI / 180.0;
   float c = std::cos(angle_radians);
   float s = std::sin(angle_radians);
   mat4 rotation_matrix = mat4::identity();
   rotation_matrix[0][0] = c; rotation_matrix[0][1] = -s;
   rotation_matrix[1][0] = s; rotation_matrix[1][1] = c;
   modelMatrix = modelMatrix * rotation_matrix;
}
void Mesh::move(vec3 move){
    modelMatrix[0][3] += move.x;
    modelMatrix[1][3] += move.y;
    modelMatrix[2][3] += move.z;
}
void Mesh::scale(vec3 scale){
for(int i:{0,1,2})
   modelMatrix[i][i] *= scale[i];
}