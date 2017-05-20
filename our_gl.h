#ifndef __OUR_GL_H__
#define __OUR_GL_H__

#include <vector>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

#define PI 3.14159265

using namespace std;

extern const TGAColor white;
extern const TGAColor red;
extern const TGAColor green;
extern const TGAColor blue;
extern const TGAColor yellow;

extern const int width;
extern const int height;
extern const int depth;

extern Model *model;

extern Matrix ModelView;
extern Matrix Viewport;
extern Matrix Projection;

Matrix v2m (Vec3f v);
Vec3f m2v (Matrix m);
vector<Vec3f> m2vs (Matrix m);
Matrix vs2m (vector<Vec3f> vs);
Matrix scale (float x_scale, float y_scale, float z_scale = 1.);
Matrix shear_x (float shear);
Matrix shear_y (float shear);
Matrix rotate (float alpha);
Matrix translate (float x_offset, float y_offset, float z_offset = 0.);

Matrix viewport (int x_offset, int y_offset, int window_width, int window_height);
Matrix projection (float c);
Matrix lookat(Vec3f eye, Vec3f center, Vec3f up);

/*
struct IShader {
    virtual ~IShader();
    virtual Vec3i vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vec3f bar, TGAColor &color) = 0;
};
//*/

void rasterize (Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color, int *ybuffer);
Vec3f barycentric (Vec3f *pts, Vec3f P);
void triangle (vector<Vec3f> pts, float *zbuffer, TGAImage &image, float intensity, Vec3f *texture_coords = NULL);
void triangle (Vec3f *pts, float *zbuffer, TGAImage &image, float intensity, Vec3f *texture_coords = NULL);
void line(Vec3i p0, Vec3i p1, TGAImage &image, TGAColor color);
void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color);

#endif
