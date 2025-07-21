#include "Scene.h"
#include "TGAImage.h"
#include "Vec.hpp"
#include "Graphic.h"
// rand()
#include <cstdlib>
#include <ctime>
/*
be careful, {X,Y,Z} is actually {right,up,left}
*/
Scene::Scene(){
   setCamera({3,3,3},{0,0,0},{0,1,0});;
   // setProjection(norm(vec3{1,1,3}));
   setProjection(50.0f,800./600.,0.1,100.0);
   setViewport(0,0,800,600);
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
    float m23 = -(zFar * zNear) / (zFar - zNear);
    float m32 = -1.0f;
    float m33 = 0.0f;

    projection = {
        vec4{m00, 0, 0, 0},   
        vec4{0, m11, 0, 0},   
        vec4{0, 0, m22, m23}, 
        vec4{0, 0, m32, m33}  
    };

    // Optional: Print the matrix for debugging
    // std::cout << "Projection Matrix:\n" << projection << std::endl;
}
void Scene::setViewport(int x,int y,int width,int height){
   // viewport = {{{width/2., 0, 0, x+width/2.}, {0, height/2., 0, y+height/2.}, {0,0,1,0}, {0,0,0,1}}};
   viewport = {
      vec4{width/2.,0,0,x+width/2},
      vec4{0,-height/2.,0,y+height/2},
      vec4{0,0,1,0},
      vec4{x+width/2.,y+height/2.,0,1}};
}

void Scene::line3D(const vec3 a,const vec3 b,TGAImage& image,const TGAColor c) const{
      vec3 ac=a,bc=b;
      for(vec3 *v:{&ac,&bc}){//used pointer to change variable
         vec4 v0 = convert_to_size<4>(*v, 1.0);
         v0 = modelview*v0;
         v0 = projection*v0;
         // if (v0[3] == 0.0f || std::fabs(v0[3]) < std::numeric_limits<float>::epsilon()) { /* std::cerr << "Clip W is zero for p0\n";*/ return; }
         v0 = v0 / v0[3];
         v0 = viewport * v0;
         *v=convert_to_size<3>(v0);
      }
      line(convert_to_size<2>(ac), convert_to_size<2>(bc), image, c);
}
void Scene::draw(TGAImage& image) const{
   srand(time(NULL));
   // #pragma omp parallel
   std::vector<float> zbuffer(image.width()*image.height());
   if(drawAxis){
      line3D(vec3{0,0,0},vec3{2,0,0},image,red);   //X
      line3D(vec3{0,0,0},vec3{0,2,0},image,green);   //Y
      line3D(vec3{0,0,0},vec3{0,0,2},image,blue);   //Z
   }
   for(const auto &mesh : Meshes){
      std::vector<vec4> draw_verts;
      for(const vec3 &vert : mesh.get_verts()){
         // object space
         vec4 v=convert_to_size<4>(vert,1.);
         // apply object matrix
         v=mesh.getModelMatrix()*v;
         // world space > view space
         v=modelview*v;
         // view space > clip space
         v=projection*v;
         // NDC(Normalized Device Coordinates) > viewport
         v=v/v[3];
         // screen space
         v=viewport*v;
         // perpective division
         //here v.z is depth and v.w=1.0
         draw_verts.push_back(v);
      }
      // drawing:
      switch (format)
      {
      case WIREFRAME:
         for( int i=0;i<mesh.nfaces();i++){
            const vec3i d=mesh.ivert(i);
            line(convert_to_size<2>(draw_verts[d[0]]),convert_to_size<2>(draw_verts[d[1]]),image,TGAColor(255,255,255));
            line(convert_to_size<2>(draw_verts[d[1]]),convert_to_size<2>(draw_verts[d[2]]),image,TGAColor(255,255,255));
            line(convert_to_size<2>(draw_verts[d[2]]),convert_to_size<2>(draw_verts[d[0]]),image,TGAColor(255,255,255));
         }
         break;
      case SOLID:
         for(int i=0;i<mesh.nfaces();i++){
            const vec3i d=mesh.ivert(i);
            TGAColor color=TGAColor(50+rand()%100,0,150+rand()%100);
            triangle3d(draw_verts[d[0]],draw_verts[d[1]],draw_verts[d[2]],image,color);
         }
         break;
      case RENDER:
         break;
      }
   }
}
void Scene::rasterize(const vec4 clip_verts[3], TGAImage &image) {

//     vec4 pts [3] = { Viewport*clip_verts[0], Viewport*clip_verts[1], Viewport*clip_verts[2] }; // screen coordinates before persp. division
//     vec2 pts2[3] = { (pts[0]/pts[0].w).xy(), (pts[1]/pts[1].w).xy(), (pts[2]/pts[2].w).xy() }; // screen coordinates after  perps. division

//     int bbminx = std::max(0, static_cast<int>(std::min(std::min(pts2[0].x, pts2[1].x), pts2[2].x))); // bounding box for the triangle
//     int bbminy = std::max(0, static_cast<int>(std::min(std::min(pts2[0].y, pts2[1].y), pts2[2].y))); // clipped by the screen
//     int bbmaxx = std::min(image.width() -1, static_cast<int>(std::max(std::max(pts2[0].x, pts2[1].x), pts2[2].x)));
//     int bbmaxy = std::min(image.height()-1, static_cast<int>(std::max(std::max(pts2[0].y, pts2[1].y), pts2[2].y)));
// #pragma omp parallel for
//     for (int x=bbminx; x<=bbmaxx; x++) { // rasterize the bounding box
//         for (int y=bbminy; y<=bbmaxy; y++) {
//             vec3 bc_screen = barycentric(pts2, {static_cast<double>(x), static_cast<double>(y)});
//             vec3 bc_clip   = { bc_screen.x/pts[0].w, bc_screen.y/pts[1].w, bc_screen.z/pts[2].w };     // check https://github.com/ssloy/tinyrenderer/wiki/Technical-difficulties-linear-interpolation-with-perspective-deformations
//             bc_clip = bc_clip / (bc_clip.x + bc_clip.y + bc_clip.z);
//             double frag_depth = bc_clip * vec3{ clip_verts[0].z, clip_verts[1].z, clip_verts[2].z };
//             if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0 || frag_depth > zbuffer[x+y*image.width()]) continue;
//             TGAColor color;
//             if (shader.fragment(bc_clip, color)) continue; // fragment shader can discard current fragment
//             zbuffer[x+y*image.width()] = frag_depth;
//             image.set(x, y, color);
//         }
//     }
}