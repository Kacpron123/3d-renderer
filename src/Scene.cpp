#include "Scene.h"
#include "TGAImage.h"
#include "Vec.hpp"
#include "Graphic.h"
// rand()
#include <cstdlib>
#include <ctime>

// be careful, {X,Y,Z} is actually {right,up,left}


Scene::Scene(){
   setCamera({3,3,3},{0,0,0},{0,1,0});;
   // setProjection(norm(vec3{1,1,3}));
   setProjection(50.0f,800./600.,0.1,100.0);
   setViewport(0,0,800,600);
}

void Scene::setCamera(const vec3 eye, const vec3 center, const vec3 up){
   vec3 z = normalized(eye - center);
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
      vec4{0,height/2.,0,y+height/2},
      vec4{0,0,1,0},
      vec4{0,0,0,1}};
   zbuffer=std::vector<std::vector<double>>(height, std::vector<double>(width, 0));
   }
   

void Scene::raster_line(vec4 p0_clip, vec4 p1_clip, TGAImage &image, TGAColor color) {
    // 1. Perform perspective divide to get NDC
    // Crucial: check for w=0 before division (point at infinity or on projection plane)
    // A robust renderer would implement line clipping in clip space here.
    // For simplicity, we'll return if w is problematic.
    if (p0_clip.w == 0.0f || std::fabs(p0_clip.w) < std::numeric_limits<float>::epsilon() ||
        p1_clip.w == 0.0f || std::fabs(p1_clip.w) < std::numeric_limits<float>::epsilon()) {
        return; 
    }

    vec4 p0_ndc = p0_clip / p0_clip.w;
    vec4 p1_ndc = p1_clip / p1_clip.w;

    // 2. Perform viewport transform to get screen-space points
    // The Z-component of pX_screen will be the NDC Z (due to viewport[2][2]=1, viewport[2][3]=0)
    vec3 p0_screen = convert_to_size<3>(viewport * p0_ndc);
    vec3 p1_screen = convert_to_size<3>(viewport * p1_ndc);

    // From here, the original Bresenham's and Z-buffer logic remains the same
    // as it now operates on the calculated screen-space coordinates (x,y)
    // and the NDC Z value (z).

    bool steep = false;
    if (std::abs(static_cast<int>(p0_screen.x) - static_cast<int>(p1_screen.x)) < std::abs(static_cast<int>(p0_screen.y) - static_cast<int>(p1_screen.y))) {
        std::swap(p0_screen.x, p0_screen.y);
        std::swap(p1_screen.x, p1_screen.y);
        steep = true;
    }
    if (p0_screen.x > p1_screen.x) {
        std::swap(p0_screen, p1_screen);
    }

    int x0 = static_cast<int>(p0_screen.x);
    int y0 = static_cast<int>(p0_screen.y);
    int x1 = static_cast<int>(p1_screen.x);
    int y1 = static_cast<int>(p1_screen.y);

    double z0 = p0_screen.z; // NDC Z from viewport transform
    double z1 = p1_screen.z; // NDC Z from viewport transform

    int dx = x1 - x0;
    int dy = y1 - y0;

    double derror2 = std::abs(dy) * 2;
    double error2 = 0;
    int y = y0;

    for (int x = x0; x <= x1; x++) {
        double t = (dx == 0) ? 0.0 : (static_cast<double>(x - x0) / dx);
        double current_z = z0 * (1.0 - t) + z1 * t;

        int current_x = steep ? y : x;
        int current_y = steep ? x : y;

        if (current_x >= 0 && current_x < image.width() && current_y >= 0 && current_y < image.height()) {
            if (current_z < zbuffer[current_y][current_x]) {
               zbuffer[current_y][current_x] = current_z;
               image.set(current_x, current_y, color);
            }
        }

        error2 += derror2;
        if (error2 > dx) {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx * 2;
        }
    }
}

void Scene::draw(TGAImage& image){
   srand(time(NULL));
   for(auto &line:zbuffer) for(double &v:line) v=std::numeric_limits<double>::max(); //clearing zbuffer

   if(drawAxis){
      double length = 2.0;
      TGAColor colors[] = {red, green, blue};  
      for (int i = 0; i < 3; i++) {
         vec4 v = {0, 0, 0, 1};
         v[i] = length;
         v = modelview * v;
         v = projection * v;
         vec4 A={0,0,0,1};
         A = modelview * A;
         A = projection * A;
         raster_line(A, v, image, colors[i]);
      }
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
         // // NDC(Normalized Device Coordinates) > viewport   ]
         // v=v/v[3];                                          ]moved to methods
         // // screen space                                    ]raster_line and rasterization
         // v=viewport*v;                                      ]
         // // perpective division
         // //here v.z is depth and v.w=1.0
         draw_verts.push_back(v);
      }
      // drawing:
      switch (format)
      {
      case WIREFRAME:
         for( int i=0;i<mesh.nfaces();i++){
            const vec3i d=mesh.ivert(i);
            raster_line(draw_verts[d[0]],draw_verts[d[1]],image,TGAColor(255,255,255));
            raster_line(draw_verts[d[1]],draw_verts[d[2]],image,TGAColor(255,255,255));
            raster_line(draw_verts[d[2]],draw_verts[d[0]],image,TGAColor(255,255,255));
         }
         break;
      case SOLID:
         for(int i=0;i<mesh.nfaces();i++){
            const vec3i d=mesh.ivert(i);
            TGAColor color=TGAColor(50+rand()%100,0,150+rand()%100);
            vec4 clip_verts[3] = { draw_verts[d[0]], draw_verts[d[1]], draw_verts[d[2]] };
            rasterize(clip_verts, image, color);
         }
         break;
      case RENDER:
         // pass
         break;
      }
   }
}
void Scene::rasterize(const vec4 clip_verts[3], TGAImage &image, TGAColor color) {
    vec4 ndc_verts[3];
    for (int i :{0,1,2}) {
        if (clip_verts[i].w == 0.0f || std::fabs(clip_verts[i].w) < std::numeric_limits<float>::epsilon()) {
            return;
        }
        ndc_verts[i] = clip_verts[i] / clip_verts[i].w;
    }

    vec4 screen_verts[3] = {
        viewport * ndc_verts[0],
        viewport * ndc_verts[1],
        viewport * ndc_verts[2]
    };

    vec2 pts2[3] = {
        convert_to_size<2>(screen_verts[0]),
        convert_to_size<2>(screen_verts[1]),
        convert_to_size<2>(screen_verts[2])
    };

    // Reszta kodu jest ok:
    int bbminx = std::max(0, static_cast<int>(std::min(std::min(pts2[0].x, pts2[1].x), pts2[2].x)));
    int bbminy = std::max(0, static_cast<int>(std::min(std::min(pts2[0].y, pts2[1].y), pts2[2].y)));
    int bbmaxx = std::min(image.width() - 1, static_cast<int>(std::max(std::max(pts2[0].x, pts2[1].x), pts2[2].x)));
    int bbmaxy = std::min(image.height() - 1, static_cast<int>(std::max(std::max(pts2[0].y, pts2[1].y), pts2[2].y)));

    #pragma omp parallel for
    for (int x = bbminx; x <= bbmaxx; x++) {
        for (int y = bbminy; y <= bbmaxy; y++) {
            vec3 bc_screen = barycentric(pts2, {static_cast<double>(x), static_cast<double>(y)});

            vec3 bc_clip = { ndc_verts[0].w * bc_screen.x, ndc_verts[1].w * bc_screen.y, ndc_verts[2].w * bc_screen.z };
            bc_clip = bc_clip / (bc_clip.x + bc_clip.y + bc_clip.z);

            double frag_depth = bc_clip * vec3{ ndc_verts[0].z, ndc_verts[1].z, ndc_verts[2].z };
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0 || frag_depth > zbuffer[y][x]) continue;

            zbuffer[y][x] = frag_depth;
            image.set(x, y, color);
        }
    }
}