#pragma once
#include <string>
#include <map>
#include "TGAImage.h"
#include "Vec.hpp"
struct Material{
   Material(): Material("None"){};
   Material(const std::string& name);
   const std::string& getName() const { return name; }
   
   std::string name;
   TGAImage diffuse_map; //(map_)Kd & color_texture
   TGAImage ambient_map; //(map_)Ka
   TGAImage specular_color_map; //(map_)Ks
   TGAImage specular_highlight_map; //(map_)Ns
   //optional
   TGAImage bump_map; //bump
   TGAImage dissolve_map; // d/Tr
   TGAImage disspacement_map; //disp
   TGAImage emission_map; //(map_)Ke

   int illnum;
   static std::map<std::string,Material> read_mtl(const std::string filename);
   private:
   static TGAImage createColor1x1(const vec3& color,TGAImage::Format format=TGAImage::RGB);
   static TGAImage createColor1x1(double grayscale_value,TGAImage::Format format=TGAImage::GRAYSCALE);
};