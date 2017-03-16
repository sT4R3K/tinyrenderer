#include "tgaimage.h"
#include <cstdlib>

void line (int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color);

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);

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
	const int dx = x1 - x0;
	const int dy = y1 - y0;

	int x;
	int y;

	for (float t = .0; t < 1.; t += 0.1) {
		x = x0 + dx*t;
		y = y0 + dy*t;
		image.set(x,y,color);
	}
} 
