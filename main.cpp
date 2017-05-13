#include <iostream>
#include <cstdlib>
#include <vector>
#include <limits>
#include <cmath>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

void rasterize (Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color, int *ybuffer);
Vec3f barycentric (Vec2f *pts, Vec3f P);
TGAColor getTextureColor (Vec2f *texture_pts, Vec3f P, float intensity);
void textured_triangle (Vec2f  *pts, Vec2f *texture_pts, TGAImage &image, float intensity);
void triangle (Vec2f *pts, TGAImage &image, TGAColor color);
void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color);

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0,   255, 0,   255);
const TGAColor blue = TGAColor(0,   0, 255,   255);
const TGAColor yellow = TGAColor(255, 255, 0,   255);

const int width  = 4800;
const int height = 4800;
const int depth  = 255;

float *zbuffer = new float[width * height];

Model *model;
TGAImage *texture = NULL;

using namespace std;

int main(int argc, char** argv) {
	if (argc == 3) {
		model = new Model (argv[1]);
		texture = new TGAImage ();
		texture->read_tga_file (argv[2]);
		texture->flip_vertically ();
	}else if (argc == 2) {
		model = new Model (argv[1]);
	} else {
		model = new Model ("obj/cube.obj");
	}

	TGAImage image (width, height, TGAImage::RGB);
	/*
	Vec3f light_dir(0,0,-1);
	for (int i=0; i<model->nfaces(); i++) { 
		std::vector<int> face = model->face(i); 
		Vec2f screen_coords[3]; 
		Vec3f world_coords[3]; 
		for (int j=0; j<3; j++) { 
			Vec3f v = model->vert(face[j]); 
			screen_coords[j] = Vec2f((v.x+1.)*width/2., (v.y+1.)*height/2.); 
			world_coords[j]  = v; 
		}
		Vec3f n = (world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0]); 
		n.normalize(); 
		float intensity = n*light_dir; 
		if (intensity>0) {
			if (texture == NULL)
				triangle(screen_coords, image, TGAColor(intensity*255, intensity*255, intensity*255, 255));
			else {
				Vec2f texture_coords[3];
				for (int i = 0; i < 3; i++) {
					Vec3f v = model->vt(face[i+3]);
					texture_coords [i] = Vec2f (v.x*texture->get_width(), v.y*texture->get_height());
				}
				textured_triangle (screen_coords, texture_coords, image, intensity); 
			}
		} 
	}
	//*/

	

	image.flip_vertically();	
	image.write_tga_file("output.tga");
	
	return 0;
}

void rasterize (Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color, int *ybuffer) {
	if (p0.x > p1.x)
		swap (p0, p1);

	int Dx = (p1.x - p0.x), Dy = (p1.y - p0.y);
	for (int x = p0.x; x <= p1.x; x++) {
		float t = (x - p0.x) / (float) Dx;
		int y = p0.y + Dy * t;
		if (ybuffer[x] < y) {
			ybuffer[x] = y;
			for (int i = 0; i <16; i++)
				image.set (x, i, color);
		}
	}
}

Vec3f barycentric (Vec2f *pts, Vec3f P) { 	// TODO: degenerate triangles.
	Vec2f A = pts[0], B = pts[1], C = pts[2];

	float D = (A.x-C.x)*(B.y-C.y) - (B.x-C.x)*(A.y-C.y);

	float alpha = ((B.y-C.y)*(P.x-C.x) + (C.x-B.x)*(P.y-C.y)) / D;
	float beta = ((C.y-A.y)*(P.x-C.x) + (A.x-C.x)*(P.y-C.y)) / D;

	return Vec3f (alpha, beta, (1. - alpha - beta));
}

TGAColor getTextureColor (Vec2f *texture_pts, Vec3f P, float intensity) {
	Vec2f T;
	for (int i=0; i<3; i++) T.x += texture_pts[i][0]*P[2-i];
	for (int i=0; i<3; i++) T.y += texture_pts[i][1]*P[2-i];
	TGAColor color = texture->get ((int ) T.x, (int) T.y);
	for (int i=0; i<3; i++) color[i] = color[i] * intensity;
	return color;
}

void textured_triangle (Vec2f  *pts, Vec2f *texture_pts, TGAImage &image, float intensity) {
	Vec2f min_max [2]; // [(min_x, min_y), (max_x, max_y)]
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			min_max[i][j] =  (i==0)? min (pts[0][j], min (pts[1][j], pts[2][j])) : max (pts[0][j], max (pts[1][j], pts[2][j]));
	Vec2f bboxmin (max (float(0), min_max[0][0]), max (float(0), min_max[0][1]));
	Vec2f bboxmax (min ((float) (image.get_width()-1), min_max[1][0]), min ((float) (image.get_height()-1), min_max[1][1]));

	Vec3f P;
	for (P.x = (int) bboxmin.x; P.x < bboxmax.x; P.x++) // Initializing P.x and P.y to floating numbers creates holes between triangles.
		for (P.y = (int) bboxmin.y; P.y < bboxmax.y; P.y++) {
			Vec3f bc_P = barycentric (pts, P);
			if (bc_P.x >= 0 &&  bc_P.y >= 0 && bc_P.z >= 0) {
				P.z = 0;
				for (int i=0; i<3; i++) P.z += pts[i][2]*bc_P[i];
            	if (zbuffer[int(P.x+P.y*width)]<P.z) {
                	zbuffer[int(P.x+P.y*width)] = P.z;
					image.set (P.x, P.y, getTextureColor (texture_pts, bc_P, intensity));
				}
			}
		}
}

void triangle (Vec2f  *pts, TGAImage &image, TGAColor color) {
	Vec2f min_max [2]; // [(min_x, min_y), (max_x, max_y)]
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			min_max[i][j] =  (i==0)? min (pts[0][j], min (pts[1][j], pts[2][j])) : max (pts[0][j], max (pts[1][j], pts[2][j]));
	Vec2f bboxmin (max (float(0), min_max[0][0]), max (float(0), min_max[0][1]));
	Vec2f bboxmax (min ((float) (image.get_width()-1), min_max[1][0]), min ((float) (image.get_height()-1), min_max[1][1]));

	Vec3f P;
	for (P.x = (int) bboxmin.x; P.x < bboxmax.x; P.x++) // Initializing P.x and P.y to floating numbers creates holes between triangles.
		for (P.y = (int) bboxmin.y; P.y < bboxmax.y; P.y++) {
			Vec3f bc_P = barycentric (pts, P);
			if (bc_P.x >= 0 &&  bc_P.y >= 0 && bc_P.z >= 0) {
				P.z = 0;
				for (int i=0; i<3; i++) P.z += pts[i][2]*bc_P[i];
            	if (zbuffer[int(P.x+P.y*width)]<P.z) {
                	zbuffer[int(P.x+P.y*width)] = P.z;
					image.set (P.x, P.y, color);
				}
			}
		}
}

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color) { 
	int Dx = p1.x - p0.x;
	int Dy = p1.y - p0.y;

	bool steep = false;
	if (abs (Dx) < abs (Dy)) {
		swap (p0.x, p0.y);
		swap (p1.x, p1.y);
		swap (Dx, Dy);
		steep = true;
	}

	if (p0.x > p1.x) {
		swap (p0.x, p1.x);
		swap (p0.y, p1.y);
		Dx *= -1;
		Dy *= -1;
	}

	int Dy2 = abs (Dy) * 2;
	int Dx2 = abs (Dx) * 2;
	int error2 = 0;

	int step = (Dy>0 ? 1 : -1);

	for (int x = p0.x, y = p0.y; x <= p1.x; x++) {
		if (steep)
			image.set (y, x, color);
		else
			image.set (x, y, color);

		error2 += Dy2;
		if (error2 > Dx) {
			y += step;
			error2 -= Dx2;
		}
	}
}