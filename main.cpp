#include <cmath>
#include <vector>
#include <iostream>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

void line (int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color);

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const int width  = 800;
const int height = 800;

using namespace std;

int main(int argc, char** argv) {
	Model *model;

	if (argc == 2) {
		model = new Model (argv[1]);
	} else {
		model = new Model ("obj/african_head.obj");
	}

	TGAImage image(width, height, TGAImage::RGB);
	for (int i=0, j; i<model->nfaces(); i++) {
		std::vector<int> face = model->face(i);
		for (j=0; j<3; j++) {
			Vec3f v0 = model->vert(face[j]);
			Vec3f v1 = model->vert(face[(j+1)%3]);
			int x0 = (v0.x+1.)*width/2.;
			int y0 = (v0.y+1.)*height/2.;
			int x1 = (v1.x+1.)*width/2.;
			int y1 = (v1.y+1.)*height/2.;
			line(x0, y0, x1, y1, image, white);
		}
	}

	image.flip_vertically(); 
	image.write_tga_file("output.tga");
	
	return 0;
}

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) { 
	int Dx = x1 - x0;
	int Dy = y1 - y0;

	bool steep = false;
	if (abs (Dx) < abs (Dy)) {
		swap (x0, y0);
		swap (x1, y1);
		swap (Dx, Dy);
		steep = true;
	}

	if (x0 > x1) {
		swap (x0, x1);
		swap (y0, y1);
		Dx *= -1;
		Dy *= -1;
	}

	int Dy2 = abs (Dy) * 2;
	int Dx2 = abs (Dx) * 2;
	int error2 = 0;

	int step = (Dy>0 ? 1 : -1);

	for (int x = x0, y = y0; x <= x1; x++) {
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
