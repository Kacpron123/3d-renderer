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
      else if(token == "o"){
         iss>>m_name;
      }
      else if (token == "v") { // vertex
         vec3 v;
         iss >> v.x >> v.y >> v.z;
         verts.push_back(v);
      }
      else if(token == "vt"){
         vec2 uv;
         iss >> uv.x >> uv.y;
         uv.y=1-uv.y; // filping y becuase obj files are inverted horizontally
         uvs.push_back(normalized(uv));
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
      faces_norms.push_back(vn);
      faces_uvs.push_back(vec2i{vt[0], vt[1]});
      }
   }
}
