// This file (tgaimage.h) is adapted from Dmitry S. Ssołow's tinyrenderer project.
// Original repository: https://github.com/ssloy/tinyrenderer
// Original author: Dmitry S. Ssołow (ssloy)

#pragma once
#include <cstdint>
#include <fstream>
#include <vector>

#pragma pack(push,1)
struct TGAHeader {
   std::uint8_t  idlength = 0;
   std::uint8_t  colormaptype = 0;
   std::uint8_t  datatypecode = 0;
   std::uint16_t colormaporigin = 0;
   std::uint16_t colormaplength = 0;
   std::uint8_t  colormapdepth = 0;
   std::uint16_t x_origin = 0;
   std::uint16_t y_origin = 0;
   std::uint16_t width = 0;
   std::uint16_t height = 0;
   std::uint8_t  bitsperpixel = 0;
   std::uint8_t  imagedescriptor = 0;
};
#pragma pack(pop)

struct TGAColor {
   /// @brief bytes per pixel
   std::uint8_t bytespp = 4;
   /// @brief red, green, blue, alpha
   union {
      struct {
         std::uint8_t b, g, r, a;
      };
      std::uint8_t bgra[4];
   };
   std::uint8_t& operator[](const int i) { return bgra[i]; }
   constexpr TGAColor():TGAColor(0){};
   constexpr TGAColor(const std::uint8_t grayscale): bgra{grayscale,grayscale,grayscale,255} {}
   constexpr TGAColor(const std::uint8_t r, const std::uint8_t g, const std::uint8_t b, const std::uint8_t a): bgra{b,g,r,a} {}
   constexpr TGAColor(const std::uint8_t r, const std::uint8_t g, const std::uint8_t b): TGAColor(r,g,b,255){}
};
// known colors:
   static constexpr TGAColor red   {255, 0,   0,   255};
   static constexpr TGAColor blue  {0,   0,   255, 255};
   static constexpr TGAColor green {0,   255, 0,   255};
   static constexpr TGAColor white {255, 255, 255, 255};
   static constexpr TGAColor black {0,   0,   0,   255};
   
   struct TGAImage {
   /// @brief color format
   enum Format { GRAYSCALE=1, RGB=3, RGBA=4 };
   TGAImage() = default;
   /// @brief create empty image
   TGAImage(const int w, const int h, const int bpp);
   /// @brief read image from file
   bool  read_tga_file(const std::string filename);
   /// @brief write image to file
   bool write_tga_file(const std::string filename, const bool vflip=true, const bool rle=true) const;
   /// @brief flip image horizontally
   void flip_horizontally();
   /// @brief flip image vertically
   void flip_vertically();
   /// @brief get color of pixel at (x,y)
   TGAColor get(const int x, const int y) const;
   /// @brief set color into pixel at  (x,y)
   void set(const int x, const int y, const TGAColor &c);
   int width()  const;
   int height() const;
   void clear(TGAColor c = black);
   void reverse();
private:
   /// @brief load rle data
   bool   load_rle_data(std::ifstream &in);
   /// @brief unload rle data
   bool unload_rle_data(std::ofstream &out) const;
   /// @brief width and height
   int w = 0, h = 0;
   /// @brief bit per pixel
   std::uint8_t bpp = 0;
   /// @brief image data
   std::vector<std::uint8_t> data = {};
};
