#pragma once
#include "Vec.hpp"
#include "TGAImage.h"

//2d:
void line(const vec2 a,const vec2 b, TGAImage &image, TGAColor color);
vec3 barycentric(const vec2 a[3], vec2 p);
/// @brief method for drawing triangle on image
/// note that verticies must be given clockwise
void triangle(vec2 a, vec2 b, vec2 c, TGAImage &image, TGAColor color);