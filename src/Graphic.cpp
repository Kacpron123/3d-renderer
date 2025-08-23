#include "Graphic.h"
void line(const vec2 a,const vec2 b, TGAImage &image, TGAColor color){
   int x1 = a.x;
   int y1 = a.y;
   int x2 = b.x;
   int y2 = b.y;
   int dx = std::abs(x2 - x1);
   int dy = std::abs(y2 - y1);
   int sx = (x1 < x2) ? 1 : -1;
   int sy = (y1 < y2) ? 1 : -1;
   int err = (dx > dy ? dx : -dy)/2, e2;

   for(;;){
      image.set(x1,y1,color);
      if (x1 == x2 && y1 == y2) break;
      e2 = err;
      if (e2 >-dx) { err -= dy; x1 += sx; }
      if (e2 < dy) { err += dx; y1 += sy; }
   }
}


vec3 barycentric(const vec2 a[3], vec2 p){
   mat<3,3> ABC={ vec3{a[0].x , a[0].y , 1.},
                  vec3{a[1].x , a[1].y , 1.},
                  vec3{a[2].x , a[2].y , 1.}};
   // if(ABC.det()<1e-9) return vec3{-1,1,1};
   return ABC.invert_transpose() * vec3{p.x,p.y,1};
}
void triangle(vec2 a, vec2 b, vec2 c, TGAImage &image, TGAColor color){
   int winding_value = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
   if(winding_value<=0) return; //if face is given  
   vec2 p[3]={a,b,c};
   // defining bounding box
   int min_x = std::min(image.width()-1, static_cast<int>(std::min(a.x, std::min(b.x, c.x))));
   int max_x = std::max(0,static_cast<int>(std::max(a.x, std::max(b.x, c.x))));
   int min_y = std::min(image.height()-1,static_cast<int>(std::min(a.y, std::min(b.y, c.y))));
   int max_y = std::max(0,static_cast<int>(std::max(a.y, std::max(b.y, c.y))));
   
   for(int x=min_x;x<=max_x;x++){
      for(int y=min_y;y<=max_y;y++){
         vec3 bary=barycentric(p,vec2{static_cast<double>(x),static_cast<double>(y)});
         if(bary[0]<0 || bary[1]<0 || bary[2]<0) continue; //pixel outside
         image.set(x,y,color);
      }
   }
}