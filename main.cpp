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

Vec3f light_dir (1,1,1);
Vec3f eye (1,1,3);
Vec3f center (0,0,0);
Vec3f up (0,1,0);

Model *model;

Matrix ModelView;
Matrix Viewport;
Matrix Projection;

struct GouraudShader : public IShader {
	Vec3f varying_intensity;
	virtual Vec4f vertex (int iface, int nthvert) {
		Vec3f normal = model->normal (iface, nthvert).normalize ();
		varying_intensity[nthvert] = normal * light_dir.normalize ();
		Vec3f v = model->vert (iface, nthvert);
		Vec4f gl_vertex = Vec4f (v.x, v.y, v.z, 1);
		return m2v4 (Viewport * Projection * ModelView * v2m (gl_vertex));
	}
	virtual bool fragment (Vec3f bc_coords, TGAColor &color) {
		float intensity = varying_intensity * bc_coords; // Interpolating intensity using a dot product.
		color = TGAColor (255, 255, 255,255) * ((intensity > 0.f)? intensity : 0.f);
		return false; // No pixel discarding !
	}
};

struct GouraudShader6 : public IShader {
	Vec3f varying_intensity;
	virtual Vec4f vertex (int iface, int nthvert) {
		Vec3f normal = model->normal (iface, nthvert).normalize ();
		varying_intensity[nthvert] = normal * light_dir.normalize ();
		Vec3f v = model->vert (iface, nthvert);
		Vec4f gl_vertex = Vec4f (v.x, v.y, v.z, 1);
		return m2v4 (Viewport * Projection * ModelView * v2m (gl_vertex));
	}
	virtual bool fragment (Vec3f bc_coords, TGAColor &color) {
		float intensity = varying_intensity * bc_coords; // Interpolating intensity using a dot product.
		if (intensity>.85) intensity = 1;
        else if (intensity>.60) intensity = .80;
        else if (intensity>.45) intensity = .60;
        else if (intensity>.30) intensity = .45;
        else if (intensity>.15) intensity = .30;
        else intensity = 0;
		color = TGAColor(255, 155, 0, 255)*intensity;
		return false; // No pixel discarding !
	}
};

struct GouraudShaderNRC : public IShader {
	Vec3f varying_intensity;
	virtual Vec4f vertex (int iface, int nthvert) {
		Vec3f normal = model->normal (iface, nthvert);
		normal = m2v ((Projection * ModelView).transpose().inverse() * v2m (normal)); // Compute the new normals of the transformed object as said in the 5th chapter.
		normal = normal.normalize ();
		varying_intensity[nthvert] = normal * light_dir.normalize ();
		Vec3f v = model->vert (iface, nthvert);
		Vec4f gl_vertex = Vec4f (v.x, v.y, v.z, 1);
		return m2v4 (Viewport * Projection * ModelView * v2m (gl_vertex));
	}
	virtual bool fragment (Vec3f bc_coords, TGAColor &color) {
		float intensity = varying_intensity * bc_coords; // Interpolating intensity using a dot product.
		color = TGAColor (255, 255, 255,255) * ((intensity > 0.f)? intensity : 0.f);
		return false; // No pixel discarding !
	}
};

struct GouraudShaderNRC6 : public IShader {
	Vec3f varying_intensity;
	virtual Vec4f vertex (int iface, int nthvert) {
		Vec3f normal = model->normal (iface, nthvert);
		normal = m2v ((Projection * ModelView).transpose().inverse() * v2m (normal)); // Compute the new normals of the transformed object as said in the 5th chapter.
		normal = normal.normalize ();
		varying_intensity[nthvert] = normal * light_dir.normalize ();
		Vec3f v = model->vert (iface, nthvert);
		Vec4f gl_vertex = Vec4f (v.x, v.y, v.z, 1);
		return m2v4 (Viewport * Projection * ModelView * v2m (gl_vertex));
	}
	virtual bool fragment (Vec3f bc_coords, TGAColor &color) {
		float intensity = varying_intensity * bc_coords; // Interpolating intensity using a dot product.
		if (intensity>.85) intensity = 1;
        else if (intensity>.60) intensity = .80;
        else if (intensity>.45) intensity = .60;
        else if (intensity>.30) intensity = .45;
        else if (intensity>.15) intensity = .30;
        else intensity = 0;
		color = TGAColor(255, 155, 0, 255)*intensity;
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
