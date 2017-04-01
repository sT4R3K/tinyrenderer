#include <cmath>
#include <vector>
#include <iostream>
#include <cstdlib>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

Vec3f barycentric (Vec2i *pts, Vec2i P);
void triangle (Vec2i *pts, TGAImage &image, TGAColor color);

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
		Vec2i screen_coords[3]; 
		Vec3f world_coords[3]; 
		for (int j=0; j<3; j++) { 
			Vec3f v = model->vert(face[j]); 
			screen_coords[j] = Vec2i((v.x+1.)*width/2., (v.y+1.)*height/2.); 
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

Vec3f barycentric (Vec2i *pts, Vec2i P) { 	// TODO: degenerate triangles.
	Vec2i A = pts[0], B = pts[1], C = pts[2];

	int D = (A.x-C.x)*(B.y-C.y) - (B.x-C.x)*(A.y-C.y);

	float alpha = ((B.y-C.y)*(P.x-C.x) + (C.x-B.x)*(P.y-C.y)) / (float) D;
	float beta = ((C.y-A.y)*(P.x-C.x) + (A.x-C.x)*(P.y-C.y)) / (float) D;

	return Vec3f (alpha, beta, (1. - alpha - beta));
}

void triangle (Vec2i  *pts, TGAImage &image, TGAColor color) {
	Vec2i min_max [2]; // [min_x, min_y, max_x, max_y]
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			min_max[i][j] =  (i==0)? min (pts[0][j], min (pts[1][j], pts[2][j])) : max (pts[0][j], max (pts[1][j], pts[2][j]));
	Vec2i bboxmin (max (0, min_max[0][0]), max (0, min_max[0][1]));
	Vec2i bboxmax (min (image.get_width()-1, min_max[1][0]), min (image.get_height()-1, min_max[1][1]));

	Vec2i P;
	for (P.x = bboxmin.x; P.x < bboxmax.x; P.x++)
		for (P.y = bboxmin.y; P.y < bboxmax.y; P.y++) {
			Vec3f bc_P = barycentric (pts, P);
			if (bc_P.x >= 0 &&  bc_P.y >= 0 && bc_P.z >= 0)
				image.set (P.x, P.y, color);
		}
}
