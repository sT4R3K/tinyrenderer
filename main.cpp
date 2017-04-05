#include <cmath>
#include <vector>
#include <iostream>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

void triangle (Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color);
void line (Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color);

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0,   255, 0,   255);

const int width  = 200;
const int height = 200;

using namespace std;

int main(int argc, char** argv) {
	TGAImage image(width, height, TGAImage::RGB);

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

void triangle (Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) {
	Vec2i t[3] = {t0,t1,t2};
	if (t[0].y > t[1].y) std::swap (t[0], t[1]);
	if (t[0].y > t[2].y) std::swap (t[0], t[2]);
	if (t[1].y > t[2].y) std::swap (t[1], t[2]);

	int Dx10 = t[1].x - t[0].x;
	int Dx20 = t[2].x - t[0].x;
	int Dx21 = t[2].x - t[1].x;

	int Dy10 = t[1].y - t[0].y;
	int Dy20 = t[2].y - t[0].y;
	int Dy21 = t[2].y - t[1].y;

	int xi = t[0].x, xj = t[0].x;
	float ti,tj;

	for (int y = t[0].y; y < t[1].y; y++) {
		ti = (y - t[0].y) / (float) Dy10;
		tj = (y - t[0].y) / (float) Dy20;
		xi = t[0].x + Dx10 * ti;
		xj = t[0].x + Dx20 * tj;
		
		if (xj < xi) swap (xi, xj);
		for (int x = xi;  x < xj; x++)
			image.set (x, y, color);
	}

	for (int y = t[1].y; y < t[2].y; y++) {
		ti = (y - t[1].y) / (float) Dy21;
		tj = (y - t[0].y) / (float) Dy20;
		xi = t[1].x + Dx21 * ti;
		xj = t[0].x + Dx20 * tj;
		
		if (xj < xi) swap (xi, xj);
		for (int x = xi;  x < xj; x++)
			image.set (x, y, color);
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
