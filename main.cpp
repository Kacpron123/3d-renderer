#include <iostream>
#include "Mesh.h"
#include "TGAImage.h"
#include "Graphic.h"
#include "Scene.h"
#include "Mesh.h"

#include <chrono>
#include <thread>

// Sleep for a specified number of milliseconds
void sleep_for_ms(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}


int main(int argc, char* argv[]) {
   
   TGAImage image(800,600,3);
   Scene scene;
   scene.setCamera({3,2,3},{0,1,0},{0,1,0});;
   // setProjection(norm(vec3{1,1,3}));
   scene.setProjection(60.0f,800./600.,0.1,100.0);
   scene.setViewport(0,0,800,600);
   scene.format=Scene::Format::SOLID;
   // Mesh Cube("../Cube.obj");
   Mesh Cube("../Cone.obj");
   scene.addMesh(Cube);
   for(int i=0;i<1;i++){
      image.clear();
      float scale=100;
      scene.getObject(0).rotateX(180);
      // scene.getObject(0).scale({scale,scale,scale});
      // scene.getObject(0).move({0,-100,0});
      scene.draw(image);
      // vec2 add={200,0};
      // triangle({100,200},{300,400},{50,300},image,red);
      // triangle(vec2{50,300}+add,vec2{300,400}+add,vec2{100,200}+add,image,blue);
      image.write_tga_file("output.tga");
      sleep_for_ms(150);
   }
   // triangle(vec2{0,0},vec2{100,0},vec2{0,100},image,TGAColor{255,0,0});
   // triangle(vec2{20,0},vec2{100,0},vec2{0,100},image,TGAColor{255,0,0});
   // Mesh Cube("../Cube.obj");
   // std::cout << Cube.nverts() << std::endl;
   // std::cout << Cube.nfaces() << std::endl;
   // for(vec3 v : Cube.get_verts()){
   //    std::cout << v << std::endl;
   // }
   return 0;
}
