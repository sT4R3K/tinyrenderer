#include "tgaimage.h"
#include <cstdlib>

void line (int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color);

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);

int main(int argc, char** argv) {
	TGAImage image(100, 100, TGAImage::RGB);

	line(13, 20, 80, 40, image, white); 

	image.flip_vertically(); 
	image.write_tga_file("output.tga");
	
	return 0;
}

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) { 
	const int Dx = x1 - x0;
	const int Dy = y1 - y0;

	float t;
	int y;
	
	for (int x = x0; x < x1; x++) {
		t = (x - x0)/(float) Dx;
		y = y0 + Dy*t;
		image.set(x, y, color);
	}
} 
