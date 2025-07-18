#include <iostream>
#include "TGAImage.h"

int main(int argc, char* argv[]) {
   constexpr int width = 800;
   constexpr int height = 600;
   TGAImage image(width, height, TGAImage::RGBA);
   struct point{int x, y;};
   point p1{100, 100}, p2{200, 200};
   TGAColor color{0, 100, 0, 255};
   TGAColor background{0, 0, 0, 255};
   for(int y = 0; y <= height; ++y) {
      for(int x = 0; x <= width; ++x) {
         if(x >= p1.x && x <= p2.x && y >= p1.y && y <= p2.y) {
            image.set(x, y, color);
         } else {
            image.set(x, y, background);
         }
      }
   }
   if(!image.write_tga_file("output.tga")) {
      std::cerr << "Failed to write TGA file\n";
      return 1;
   }
   return 0;
}
