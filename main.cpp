#include <cmath>

#include "tgaimage.h"

void line (int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color);

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);

using namespace std;

int main(int argc, char** argv) {
	TGAImage image(100, 100, TGAImage::RGB);

	line(13, 20, 80, 40, image, white); 
	line(20, 13, 40, 80, image, red); 
	line(80, 40, 13, 20, image, red);

	image.flip_vertically(); 
	image.write_tga_file("output.tga");
	
	return 0;
}

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) { 
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
			y += (Dy>0 ? 1 : -1);
			error2 -= Dx2;
		}
	}
} 
