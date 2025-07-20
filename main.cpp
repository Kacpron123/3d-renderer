#include <iostream>
#include "Mesh.h"
#include "TGAImage.h"
#include "Graphic.h"
#include "Scene.h"
#include "Mesh.h"
int main(int argc, char* argv[]) {
   
   TGAImage image(800,600,3);
   Scene scene;
   Mesh Cube("../Cube.obj");
   scene.addMesh(Cube);
   scene.draw(image);
   // triangle(vec2{0,0},vec2{100,0},vec2{0,100},image,TGAColor{255,0,0});
   // triangle(vec2{20,0},vec2{100,0},vec2{0,100},image,TGAColor{255,0,0});
   image.write_tga_file("output.tga");
   // Mesh Cube("../Cube.obj");
   // std::cout << Cube.nverts() << std::endl;
   // std::cout << Cube.nfaces() << std::endl;
   // for(vec3 v : Cube.get_verts()){
   //    std::cout << v << std::endl;
   // }
   return 0;
}
