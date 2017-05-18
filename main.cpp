#include <iostream>
#include <cstdlib>
#include <vector>
#include <limits>
#include <cmath>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"

using namespace std;

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0,   255, 0,   255);
const TGAColor blue = TGAColor(0,   0, 255,   255);
const TGAColor yellow = TGAColor(255, 255, 0,   255);

const int width  = 1600;
const int height = 1600;
const int depth  = 255;

Vec3f light_dir (0,0,-1);
Vec3f camera (0,0,3);
Vec3f eye (1,2,3);
Vec3f center (0,0,0);
Vec3f up (0,1,0);

Model *model;
TGAImage *texture = NULL;

Matrix ModelView;
Matrix Viewport;
Matrix Projection;

int main(int argc, char** argv) {
	if (argc == 3) {
		model = new Model (argv[1]);
		texture = new TGAImage ();
		texture->read_tga_file (argv[2]);
		texture->flip_vertically ();
	}else if (argc == 2) {
		model = new Model (argv[1]);
	} else {
		model = new Model ("obj/african_head.obj");
	}

	float *zbuffer = new float[width * height];
	for (int i=0; i<width*height; i++) {
        zbuffer[i] = std::numeric_limits<float>::min();
    }

	TGAImage image (width, height, TGAImage::RGB);

	ModelView = lookat (eye, center, up);
	Viewport = viewport (width/9., height/9., 7*width/9., 7*height/9.);
	Projection = projection (-1./(eye-center).norm());

	for (int i=0; i<model->nfaces(); i++) { 
		vector<int> face = model->face(i); 
		vector<Vec3f> screen_coords; 
		vector<Vec3f> world_coords; 
		for (unsigned int j=0; j<face.size()/2; j++) // (size/2) because the second half contains vt(s).
			world_coords.push_back (model->vert(face[j])); 
		
		screen_coords = m2vs (Viewport * Projection * ModelView * vs2m (world_coords));

		Vec3f n = (world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0]); 
		n.normalize(); 
		float intensity = n*light_dir;

		if (intensity>0) {
			if (texture == NULL)
				triangle(screen_coords, image, TGAColor(intensity*255, intensity*255, intensity*255, 255), zbuffer);
			else {
				Vec2f texture_coords[3];
				for (unsigned int k = 0; k < face.size()/2; k++) {
					Vec3f v = model->vt(face[k+(face.size()/2)]);
					texture_coords [k] = Vec2f (v.x*texture->get_width(), v.y*texture->get_height());
				}
				textured_triangle (screen_coords, texture_coords, image, intensity, zbuffer); 
			}
		} 
	}

	image.flip_vertically();	
	image.write_tga_file("output.tga");
	
	return 0;
}
