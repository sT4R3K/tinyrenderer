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

Vec3f light_dir (1,-1,1);
Vec3f eye (1,1,3);
Vec3f center (0,0,0);
Vec3f up (0,1,0);

Model *model;

Matrix ModelView;
Matrix Viewport;
Matrix Projection;

struct GouraudShader : public IShader {
	Vec3f varying_intensity;
	Vec3f varying_texture [3];

	virtual Vec4f vertex (int iface, int nthvert) {
		Vec3f normal = model->normal (iface, nthvert);
		normal = m2v ((Projection).transpose().inverse() * v2m (normal)); // Compute the new normals of the transformed object as said in the 5th chapter.
		normal = normal.normalize ();
		varying_intensity[nthvert] = normal * light_dir.normalize ();
		varying_texture[nthvert] = model->vt (iface, nthvert);
		Vec3f v = model->vert (iface, nthvert);
		Vec4f gl_vertex = Vec4f (v.x, v.y, v.z, 1);
		return m2v4 (Viewport * Projection * ModelView * v2m (gl_vertex));
	}
	virtual bool fragment (Vec3f bc_coords, TGAColor &color) {
		float intensity = varying_intensity * bc_coords; // Interpolating intensity using a dot product.
		if (!model->has_texture ())
			color = TGAColor (255, 255, 255, 255) * ((intensity > 0.f)? intensity : 0.f);
		else
			color = model->getTextureColor (varying_texture, bc_coords) * ((intensity > 0.f)? intensity : 0.f); // Model already knows how to interpolate texture coordinates.
		return false; // No pixel discarding !
	}
};

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
	GouraudShader *shader = new GouraudShader ();
	for (int i = 0; i < model->nfaces(); i++) { 
		Vec4f screen_coords[3];
		for (int k = 0; k < 3; k++)
			screen_coords[k] = shader->vertex (i, k);
		triangle(screen_coords, shader, zbuffer, image);

		if (i == 0) continue;
		else if ((i+1)/model->nfaces () == 1) cout << "100%" << endl;
		else if ((i+1) % int (roundf (model->nfaces ()/10)) == 0) cout << (100 * i / model->nfaces ()) << "%...";
	}

	image.flip_vertically();
	cout << "Wrighting to output.tga..." << endl;
	image.write_tga_file("output.tga");
	cout << "OK" << endl;
	
	return 0;
}
