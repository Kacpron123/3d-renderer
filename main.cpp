#include <iostream>
#include "TGAImage.h"
#include "Graphic.h"
#include "Vec.hpp"

// added for looking and changes
#include <thread>
#include <chrono>

int main(int argc, char* argv[]) {
   constexpr int width = 800;
   constexpr int height = 600;
   TGAImage image(width, height, TGAImage::RGBA);
   TGAColor green{50, 200, 50, 255};
   TGAColor blue{200, 50, 50, 255};
   TGAColor red{50, 50, 200, 255};

   TGAColor background{0, 0, 0, 255};
   for(int y = 0; y <= height; ++y) {
      for(int x = 0; x <= width; ++x)
         image.set(x, y, background);
   }
   vec2 p3[3]={{50, 60}, {80, 160}, {100, 100}};
   vec2 add{-20,40};

   for(int i=0;i<100;i++){
      image.clear();
      triangle(p3[0]+add+vec2{i,i},p3[1]+add,p3[2]+add, image, red);
      if(!image.write_tga_file("output.tga")) {
         std::cerr << "Failed to write TGA file\n";
         return 1;
      }
      // break;
      
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
   }
   return 0;
}
