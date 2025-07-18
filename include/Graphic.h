#pragma once
#include "Point.hpp"
#include "TGAImage.h"

void viewport();
void camera(const vec3 eye, const vec3 center, const vec3 up);

void line(const vec2 p1,const vec2 p2, TGAImage &image, TGAColor color){
    int x1 = p1.x;
    int y1 = p1.y;
    int x2 = p2.x;
    int y2 = p2.y;
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

void triangle(vec2 p1, vec2 p2, vec2 p3, TGAImage &image, TGAColor color){
   if(p1.y>p2.y) std::swap(p1,p2);
   if(p1.y>p3.y) std::swap(p1,p3);
   if(p2.y>p3.y) std::swap(p2,p3);
   int total_hight=p3.y-p1.y;
   // lower half
   if(p1.y!=p2.y){
      int segment_hight=p2.y-p1.y;
      for(int y=p1.y;y<=p2.y;y++){
         int x1=p1.x+((p3.x-p1.x)*(y-p1.y))/total_hight;
         int x2=p2.x+((p2.x-p1.x)*(y-p2.y))/segment_hight;         
         for(int x=std::min(x1,x2);x<std::max(x1,x2);x++)
            image.set(x,y,color);
      }
   }
   // upper half
   if(p3.y!=p2.y){
      int segment_hight=p3.y-p2.y;
      for(int y=p2.y;y<=p3.y;y++){
         int x1=p3.x+((p3.x-p1.x)*(y-p3.y))/total_hight;
         int x2=p2.x+((p3.x-p2.x)*(y-p2.y))/segment_hight;         
         for(int x=std::min(x1,x2);x<std::max(x1,x2);x++)
            image.set(x,y,color);
      }
   }
}
