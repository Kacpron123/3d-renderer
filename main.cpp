#include <iostream>
#include <chrono>
#include <thread>

#include "Mesh.h"
#include "TGAImage.h"
#include "Graphic.h"
#include "Scene.h"
#include <memory>
#include "Tracer.cuh"

// Sleep for a specified number of milliseconds
void sleep_for_ms(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}


int main(int argc, char* argv[]) {
   double times=1;
   int width=800*times,height=600*times;
   TGAImage image(width,height,3);
   Scene scene;
   scene.setCamera({2.5,3,2.5},{0,1.5,0},{0,1,0});;
   // // setProjection(norm(vec3{1,1,3}));
   scene.setProjection(60.0f,(double)width/height,0.1,100.0);
   scene.setViewport(0,0,width,height);
   scene.addLight({4,5,2},2);
   scene.addLight({0,5,2},1);
   scene.format=Scene::Format::RENDER;
   std::shared_ptr<Mesh> Crystal = std::make_shared<Mesh>("../obj/Crystal/Crystals_low.obj");
   std::shared_ptr<Mesh> Cube = std::make_shared<Mesh>("../obj/Cube.obj");
   std::shared_ptr<Mesh> Cube2 = std::make_shared<Mesh>("../obj/Cube.obj");
   Cube->move(vec3{1,1,1});
   Cube2->scale(0.5);
   // Cube2->move(vec3{2,3,2});
   
   Crystal->move(vec3{1.5,2,1});
   // Crystal->scale(2);
   scene.addMesh(Crystal);
   scene.addMesh(Cube);
   // scene.addMesh(Cube2);
   for(int i=0;i<1;i++){
      image.clear();
      // Crystal->rotateX(10);
      // scene.getObject(0).rotateY(10);
      scene.draw(image);
      image.write_tga_file("output.tga");
      // sleep_for_ms(300);
   }
   // image.write_tga_file("output.tga");
   
   return 0;
}