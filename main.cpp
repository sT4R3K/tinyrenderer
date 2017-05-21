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

const int width  = 4800;
const int height = 4800;
const int depth  = 255;

Vec3f light_dir (0,0,-1);
Vec3f eye (1,1,3);
Vec3f center (0,0,0);
Vec3f up (0,1,0);

Model *model;

Matrix ModelView;
Matrix Viewport;
Matrix Projection;

int main(int argc, char** argv) {
	if (argc == 3) {
		model = new Model (argv[1], argv[2]);
	}else if (argc == 2) {
		model = new Model (argv[1]);
	} else {
		model = new Model ("obj/african_head.obj");
	}

	float *zbuffer = new float[width * height];
	for (int i = 0; i < width * height; i++)
		zbuffer[i] = -numeric_limits<float>::infinity();

	TGAImage image (width, height, TGAImage::RGB);

	Viewport = viewport (width/9., height/9., 7*width/9., 7*height/9.);
	Projection = projection (-1./(eye-center).norm());
	ModelView = lookat (eye, center, up);

	cout << "Rendering...";
	for (int i = 0; i < model->nfaces(); i++) { 
		vector<Vec3i> face = model->face(i); 
		vector<Vec3f> screen_coords; 
		vector<Vec3f> world_coords; 
		for (unsigned int k = 0; k < face.size(); k++)
			world_coords.push_back (model->vert(face[k][0])); 
		
		screen_coords = m2vs (Viewport * Projection * ModelView * vs2m (world_coords));
		// Matrix Normal_transformer = (Projection * ModelView).transpose().inverse();

		Vec3f n = (world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0]); 
		n.normalize(); 
		float intensity = n*light_dir;

		if (intensity>0) {
			if (! model->has_texture ())
				triangle(screen_coords, zbuffer, image, intensity);
			else {
				Vec3f texture_coords[3];
				for (int k = 0; k < 3; k++)
					texture_coords [k]= model->vt(face[k][1]);
				triangle(screen_coords, zbuffer, image, intensity, texture_coords);
			}
		}

		if (i == 0) continue;
		else if ((i+1)/model->nfaces () == 1) cout << "100%" << endl;
		else if ((i+1) % int (roundf (model->nfaces ()/10)) == 0) cout << roundf (100 * i / model->nfaces ()) << "%...";
	}

	image.flip_vertically();
	cout << "Wrighting to output.tga..." << endl;
	image.write_tga_file("output.tga");
	cout << "OK" << endl;
	
	return 0;
}
