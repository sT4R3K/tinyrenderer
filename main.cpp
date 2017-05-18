#include <cmath>
#include <vector>
#include <iostream>
#include <cstdlib>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

Vec3f barycentric (Vec2f *pts, Vec2f P);
void triangle (Vec2f *pts, TGAImage &image, TGAColor color);
void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color);

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0,   255, 0,   255);

const int width  = 3200;
const int height = 3200;

using namespace std;

int main(int argc, char** argv) {
	Model *model;

	if (argc == 2) {
		model = new Model (argv[1]);
	} else {
		model = new Model ("obj/african_head.obj");
	}

	TGAImage image(width, height, TGAImage::RGB);
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
			triangle(screen_coords, image, TGAColor(intensity*255, intensity*255, intensity*255, 255)); 
		} 
	}

	image.flip_vertically(); 
	image.write_tga_file("output.tga");
	
	return 0;
}

Vec3f barycentric (Vec2f *pts, Vec2f P) {
	Vec2f A = pts[0], B = pts[1], C = pts[2];

	float D = (A.x-C.x)*(B.y-C.y) - (B.x-C.x)*(A.y-C.y);
	if (D == 0) return Vec3f (-1,-1,-1); // Getting rid of degenerate triangles.

	float alpha = ((B.y-C.y)*(P.x-C.x) + (C.x-B.x)*(P.y-C.y)) / D;
	float beta = ((C.y-A.y)*(P.x-C.x) + (A.x-C.x)*(P.y-C.y)) / D;

	return Vec3f (alpha, beta, (1. - alpha - beta));
}

void triangle (Vec2f  *pts, TGAImage &image, TGAColor color) {
	Vec2f min_max [2]; // [min_x, min_y, max_x, max_y]
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			min_max[i][j] =  (i==0)? fmin (pts[0][j], fmin (pts[1][j], pts[2][j])) : fmax (pts[0][j], fmax (pts[1][j], pts[2][j]));
	Vec2f bboxmin (fmax (0, min_max[0][0]), fmax (0, min_max[0][1]));
	Vec2f bboxmax (fmin (float(image.get_width()-1), min_max[1][0]), fmin (float(image.get_height()-1), min_max[1][1]));

	Vec2f P;
	for (P.x = roundf (bboxmin.x); P.x <= roundf (bboxmax.x); P.x++)
		for (P.y = roundf (bboxmin.y); P.y <= roundf (bboxmax.y); P.y++) {
			Vec3f bc_P = barycentric (pts, P);
			if (bc_P.x >= 0 &&  bc_P.y >= 0 && bc_P.z >= 0)
				image.set (P.x, P.y, color);
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