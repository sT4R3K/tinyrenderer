#include <cmath>
#include <vector>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

void line (int, int, int, int, TGAImage&, TGAColor);
void triangle (Vec2i, Vec2i, Vec2i, TGAImage&, TGAColor);

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0,   255, 0,   255);

const int width  = 260;
const int height = 260;

using namespace std;

int main(int argc, char** argv) {
	Model *model;

	if (argc == 2) {
        model = new Model (argv[1]);
    } else {
        model = new Model ("obj/african_head.obj");
    }

	TGAImage image(width, height, TGAImage::RGB);

    /*
    for (int i=0; i<model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        for (int j=0; j<3; j++) {
            Vec3f v0 = model->vert(face[j]);
            Vec3f v1 = model->vert(face[(j+1)%3]);
            int x0 = (v0.x+1.)*width/2.;
            int y0 = (v0.y+1.)*height/2.;
            int x1 = (v1.x+1.)*width/2.;
            int y1 = (v1.y+1.)*height/2.;
            line(x0, y0, x1, y1, image, white);
        }
    }
	//*/
    /*
		line (400,720,20,20,image,red);
		line (20,20,600,60,image,red);
	    line (600,60,400,720,image,red);
	//*/

	Vec2i t0[3] = {Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)}; 
	Vec2i t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)}; 
	Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)}; 
	triangle(t0[0], t0[1], t0[2], image, red); 
	triangle(t1[0], t1[1], t1[2], image, white); 
	triangle(t2[0], t2[1], t2[2], image, green);

	image.flip_vertically(); 
	image.write_tga_file("output.tga");
	
	return 0;
}

void line (int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) { 
	int Dx = abs (x1 - x0);
	int Dy = abs (y1 - y0);

	bool steep = false;
	if (Dx < Dy) {
		swap (x0, y0);
		swap (x1, y1);
		swap (Dx, Dy);
		steep = true;
	}

	if (x0 > x1) {
		swap (x0, x1);
		swap (y0, y1);
	}

	int Dy2 = abs (Dy) * 2;
	int Dx2 = abs (Dx) * 2;
	float error2 = 0;

	int y = y0;

	for (int x = x0; x <= x1; x++) {
		if (steep)
			image.set (y, x, color);
		else
			image.set (x, y, color);

		error2 += Dy2;
		if (error2 > Dx) {
			y += (y1>y0 ? 1 : -1);
			error2 -= Dx2;
		}
	}
} 

void triangle (Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) {
	line (t0.x, t0.y, t1.x, t1.y, image, color);
	line (t1.x, t1.y, t2.x, t2.y, image, color);
	line (t2.x, t2.y, t0.x, t0.y, image, color);

	
}