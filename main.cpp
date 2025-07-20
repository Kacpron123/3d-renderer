#include <iostream>
#include "Mesh.h"


int main(int argc, char* argv[]) {
   
   Mesh Cube("../Cube.obj");
   std::cout << Cube.nverts() << std::endl;
   std::cout << Cube.nfaces() << std::endl;
   for(vec3 v : Cube.get_verts()){
      std::cout << v << std::endl;
   }
   return 0;
}
