#include <iostream>
#include "TGAImage.h"
#include "Graphic.h"
#include "Point.hpp"

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
   triangle(p3[0],p3[1],p3[2], image, green);
   if(!image.write_tga_file("output.tga")) {
      std::cerr << "Failed to write TGA file\n";
      return 1;
   }
   return 0;
}
