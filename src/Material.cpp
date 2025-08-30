#include "Material.h"
#include <fstream>
#include <iostream>
#include <sstream>

TGAImage Material::createColor1x1(const vec3& color,TGAImage::Format format){
   TGAImage img(1, 1, format);
   TGAColor tga_color(
         static_cast<std::uint8_t>(color.x * 255.0f),
         static_cast<std::uint8_t>(color.y * 255.0f),
         static_cast<std::uint8_t>(color.z * 255.0f)
   );
   img.set(0, 0, tga_color);
   return img;
}
TGAImage Material::createColor1x1(double grayscale_value,TGAImage::Format format){
   TGAImage img(1,1,format);
   img.set(0,0,TGAColor(grayscale_value*255.0f));
   return img;
}

Material::Material(const std::string& name) : name(name) {
   // default values
   specular_highlight_map=createColor1x1(.5);
   ambient_map=createColor1x1(vec3{.8,.8,.8});
   diffuse_map=createColor1x1(vec3{.8,.8,.8});
   specular_color_map=createColor1x1(vec3{.8,.8,.8});
   dissolve_map=createColor1x1(1.0f);
   bump_map=createColor1x1(vec3{0.5,0.5,1.0});
   disspacement_map=createColor1x1(0.5);
   emission_map=createColor1x1(0.0);
   
}

std::map<std::string,Material> Material::read_mtl(const std::string filename){
   std::map<std::string, Material> result;
   std::ifstream file(filename);
   if (!file.is_open()) { // can't open file
      std::cerr << "Error: Could not open MTL file: " << filename << ". Returning default material.\n";
      result["None"]=Material("None");
      return result;
   }
   std::string path = filename.substr(0, filename.find_last_of('/'))+"/";

   std::string line;
   Material current_material("");
   while (std::getline(file, line)) {
      std::istringstream iss(line);
      std::string token;
      iss >> token;
      if (token.empty() || token[0] == '#') {
         continue; // Skip empty lines and comments
      }
      if (token == "newmtl") {
         if (!current_material.name.empty()) { // If we were building a material, save it
               // Check for duplicates names
               if (result.count(current_material.name)) {
                  std::cerr << "Warning: Duplicate material name '" << current_material.name << "' in MTL file. Overwriting.\n";
               }
               result[current_material.name] = current_material;
         }
         // Start a new material
         current_material = Material(""); // Reset all to defaults
         std::string mat_name;
         iss >> mat_name;
         current_material.name=mat_name;
      }
      
      // --- Colors ---
      else if (token == "Ka") { // Ambient color
         vec3 color;
         iss >> color.x >> color.y >> color.z;
         current_material.ambient_map=createColor1x1(color);
      }
      else if (token == "Kd") { // Diffuse color
         vec3 color;
         iss >> color.x >> color.y >> color.z;
         current_material.diffuse_map=createColor1x1(color);
      }
      else if (token == "Ks") { // Specular color
         vec3 color;
         iss >> color.x >> color.y >> color.z;
         current_material.specular_color_map=createColor1x1(color);
      }
      else if (token == "Ke") { // Emission color (non-standard but useful)
         vec3 color;
         iss >> color.x >> color.y >> color.z;
         current_material.emission_map=createColor1x1(color);
      }
      // --- Scalar Properties ---
      else if (token == "Ns") { // Shininess (specular exponent)
         double shininess_val;
         iss >> shininess_val;
         current_material.specular_highlight_map=createColor1x1(shininess_val);
      }
      else if (token == "d" || token == "Tr") { // Dissolve (opacity)
         double dissolve_val;
         iss >> dissolve_val;
         if(token=="Tr") dissolve_val=1-dissolve_val; //reverse
         current_material.dissolve_map=createColor1x1(dissolve_val);
      }
      // else if (token == "Ni") { // Optical Density / Index of Refraction
      //    double ni_val;
      //    iss >> ni_val;
      //    current_material.setOpticalDensity(ni_val);
      // }
      // else if (token == "illum") { // Illumination model
      //    int illum_val;
      //    iss >> illum_val;
      //    current_material.setIllumModel(illum_val);
      // }

      // --- Textures ---
      else if (token == "map_Ka") { //ambient texture
         std::string texture_path;
         iss >> texture_path;
         TGAImage img;
         if(img.read_tga_file(path+texture_path))
            current_material.ambient_map=img;
      }
      else if (token == "map_Kd") { //diffuse texture
         std::string texture_path;
         iss >> texture_path;
         TGAImage img;
         if(img.read_tga_file(path+texture_path))
            current_material.diffuse_map=img;
      }
      else if (token == "map_Ks") { //specular_color_texture
         std::string texture_path;
         iss >> texture_path;
         TGAImage img;
         if(img.read_tga_file(path+texture_path))
            current_material.specular_color_map=img;
      }
      else if (token == "map_Ns") { // Specular highlight map
         std::string texture_path;
         iss >> texture_path;
         TGAImage img;
         if(img.read_tga_file(path+texture_path))
            current_material.specular_highlight_map=img;
      }
      else if (token == "map_Bump" || token == "bump") { // Normal/Bump Map
         std::string texture_path;
         iss >> texture_path;
         TGAImage img;
         if(img.read_tga_file(path+texture_path))
            current_material.bump_map=img;
         // Handle optional arguments like -bm for bump strength if needed
      }
      else if (token == "map_d" || token == "map_Tr") { // Dissolve/Transparency Map
         std::string texture_path;
         iss >> texture_path;
         TGAImage img;
         if(img.read_tga_file(path+texture_path)){
            if(token=="map_Tr") img.reverse();
            current_material.dissolve_map=img;
         }
      }
      else if (token == "map_disp" || token == "disp") { // Displacement Map
         std::string texture_path;
         iss >> texture_path;
         TGAImage img;
         if(img.read_tga_file(path+texture_path))
            current_material.disspacement_map=img;
      }
      else if (token == "map_Ke") { // Emission Map (custom)
         std::string texture_path;
         iss >> texture_path;
         TGAImage img;
         if(img.read_tga_file(path+texture_path))
            current_material.emission_map=img;
      }
   }
   // After the loop, add the last material that was being built
   if (!current_material.name.empty()) {
   result[current_material.name] = current_material;
   }

   // Ensure there's at least one default material if the file was empty or had issues
   if (result.empty()) {
      Material defaultMat("None");
      result["None"] = defaultMat;
   }
   return result;
}