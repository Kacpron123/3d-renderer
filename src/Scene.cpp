#include "Scene.h"
#include "TGAImage.h"
#include "Vec.hpp"
#include "Graphic.h"
#include <cstdlib>
#include <ctime>
Scene::Scene(){
   setCamera({10,10,10},{0,0,0},{0,0,1});;
   // setProjection(norm(vec3{1,1,3}));
   setProjection(100.0f,800/600,0.1,100.0);
   setViewport(100,100,800,600);
}

void Scene::setCamera(const vec3 eye, const vec3 center, const vec3 up){
   vec3 z = normalized(center - eye);
   vec3 x = normalized(cross(up,z));
   vec3 y = cross(z,x);
   mat4 A = mat4{convert_to_size<4>(x),convert_to_size<4>(y),convert_to_size<4>(z),vec4{0,0,0,1}};
   mat4 B = once<4>();
   B[0][3] += -eye.x; B[1][3] += -eye.y; B[2][3] += -eye.z; 
   modelview = A*B;
}
void Scene::setProjection(float f){
   projection = {vec4{1,0,0,0},vec4{0,-1,0,0},vec4{0,0,1,0},vec4{0,0,-1/f,0}};
}
void Scene::setProjection(float fovy_degrees, float aspect, float zNear, float zFar) {
    // Convert field of view from degrees to radians, as tan() expects radians.
    float fovy_radians = fovy_degrees * M_PI / 180.0f;
    float tanHalfFovy = tan(fovy_radians / 2.0f);

    float m00 = 1.0f / (aspect * tanHalfFovy);
    float m11 = 1.0f / tanHalfFovy;
    float m22 = -(zFar + zNear) / (zFar - zNear);
    float m23 = -(2.0f * zFar * zNear) / (zFar - zNear);
    float m32 = -1.0f;
    float m33 = 0.0f;

    projection = {
        vec4{m00, 0, 0, 0},   
        vec4{0, m11, 0, 0},   
        vec4{0, 0, m22, m32}, 
        vec4{0, 0, m23, m33}  
    };

    // Optional: Print the matrix for debugging
    // std::cout << "Projection Matrix:\n" << projection << std::endl;
}
void Scene::setViewport(int x,int y,int width,int height){
   viewport = {
      vec4{width/2.,0,0,0},
      vec4{0,-height/2.,0,0},
      vec4{0,0,1,0},
      vec4{x+width/2.,y+height/2.,0,1}};
}
void Scene::draw(TGAImage& image) const{
   srand(time(NULL));
   // #pragma omp parallel
   for(const auto &mesh : Meshes){
      std::vector<vec2> draw_verts;
      for(const vec3 &vert : mesh.get_verts()){
         // object space
         vec4 v=convert_to_size<4>(vert,1.);
         // object space > view space
         v=modelview*v;
         // view space > clip space
         v=projection*v;
         // NDC(Normalized Device Coordinates) > viewport
         v=v/v[3];
         // screen space
         v=viewport*v;
         // perpective division
         draw_verts.push_back(convert_to_size<2>(v));
      }
      for(vec d:draw_verts)
         std::cout<<d<<std::endl;
      std::cout<<mesh.nfaces()<<std::endl;
      for( int i=0;i<mesh.nfaces();i++){
         const vec3i d=mesh.ivert(i);
         vec2 add=vec2{(double)image.width(),(double)image.height()}/2.0;
         line(draw_verts[d[0]]+add,draw_verts[d[1]]+add,image,TGAColor((rand()&255),(rand()&255),(rand()&255)));
         line(draw_verts[d[1]]+add,draw_verts[d[2]]+add,image,TGAColor((rand()&255),(rand()&255),(rand()&255)));
         line(draw_verts[d[2]]+add,draw_verts[d[0]]+add,image,TGAColor((rand()&255),(rand()&255),(rand()&255)));
      }
   }
}