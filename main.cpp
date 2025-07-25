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
   double times=1;
   int width=800*times,height=600*times;
   TGAImage image(width,height,3);
   Scene scene;
   scene.setCamera({2.5,3,2.5},{0,1.5,0},{0,1,0});;
   // // setProjection(norm(vec3{1,1,3}));
   scene.setProjection(60.0f,(double)width/height,0.1,100.0);
   scene.setViewport(0,0,width,height);
   scene.format=Scene::Format::RENDER;
   Mesh Crystal("../obj/Crystal/Crystals_low.obj");
   Crystal.scale(3);
   scene.addMesh(Crystal);
   scene.draw(image);
   image.write_tga_file("output.tga");
   
   return 0;
}