#include <iostream>
#include <cstdlib>
#include <vector>
#include <limits>
#include <cmath>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

#define PI 3.14159265

using namespace std;

Matrix v2m (Vec3f v);
Vec3f m2v (Matrix m);
vector<Vec3f> m2vs (Matrix m);
Matrix vs2m (vector<Vec3f> vs);
Matrix scale (float x_scale, float y_scale, float z_scale = 1.);
Matrix shear_x (float shear);
Matrix shear_y (float shear);
Matrix rotate (float alpha);
Matrix translate (float x_offset, float y_offset, float z_offset = 1.);
Matrix viewport (int x_offset, int y_offset, int window_width, int window_height);
Matrix projection (float p);
vector<Vec3f> central_projection (vector<Vec3f> pts, Vec3f camera);

void rasterize (Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color, int *ybuffer);
Vec3f barycentric (Vec3f *pts, Vec3f P);
void triangle (Vec3f *pts, float *zbuffer, TGAImage &image, float intensity, Vec3f *texture_coords = NULL);
void line(Vec3i p0, Vec3i p1, TGAImage &image, TGAColor color);
void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color);

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0,   255, 0,   255);
const TGAColor blue = TGAColor(0,   0, 255,   255);
const TGAColor yellow = TGAColor(255, 255, 0,   255);

const int width  = 1500;
const int height = 600;
const int depth  = 255;

Model *model;

int main(int argc, char** argv) {
	if (argc == 3) {
		model = new Model (argv[1], argv[2]);
	}else if (argc == 2) {
		model = new Model (argv[1]);
	} else {
		model = new Model ("obj/cube.obj");
	}

	TGAImage image1 (width, height, TGAImage::RGB);
	TGAImage image2 (width, height, TGAImage::RGB);
	
	Matrix VP = viewport (height/8., height/8., 3*height/4., 3*height/4.);

	// Extracting the model vertices into a matrix:
	vector<Vec3i> face = model->face(0);
	vector<Vec3f> face_verts;
	for (unsigned int i = 0; i < face.size(); i ++)
		face_verts.push_back (model->vert(face[i][0]));	
	Matrix face_matrix = vs2m (face_verts);

	// Drawing the white vertical line:
	line (m2v(VP*v2m(Vec3f(0,2,0))), m2v(VP*v2m(Vec3f(0,-2,0))), image1, white);
	line (m2v(VP*v2m(Vec3f(0,2,0))), m2v(VP*v2m(Vec3f(0,-2,0))), image2, white);

	// Drawind the yellow lines
	Vec3f v0 = m2v (VP * v2m (Vec3f(5,0,0)));
	for (unsigned int i = 0; i<face_verts.size(); i++) {
		Vec3f v1 = face_verts[i];
		if (v1.x > 0) { // If the line doesn't reah the screen line (white vertical).
			float a = (v1.y)/(v1.x-5);
			float b = v1.y - (a * v1.x);
			v1.x = 0; v1.y = b;
		}
		v1 = m2v (VP * v2m (v1));
		line (v0, v1, image1, yellow);
		line (v0, v1, image2, yellow);
	}
	
	// Drawing the axes:
	Vec3f o(0,0,0), x(1,0,0), y(0,1,0);
	vector<Vec3f> axes;
	axes.push_back (o);
	axes.push_back (x);
	axes.push_back (y);
	axes = m2vs (VP * vs2m(axes));
	line (axes[0], axes[1], image1, red);
	line (axes[0], axes[1], image2, red);
	line (axes[0], axes[2], image1, green);
	line (axes[0], axes[2], image2, green);

	// Drawing the original model:
	vector<Vec3f> screen_face_verts = m2vs (VP * face_matrix);
	for (unsigned int i = 0; i<screen_face_verts.size(); i++) {
		Vec3f v0 = screen_face_verts[i];
		Vec3f v1 = screen_face_verts[(i+1)%screen_face_verts.size()];
		line (v0,v1, image1, white);
		line (v0,v1, image2, white);
	}

	// Drawing the deformed model using a matrix (projection ()):
	vector<Vec3f> deformed_face_verts = m2vs (VP * projection (-1./5) * face_matrix);
	for (unsigned int i = 0; i<deformed_face_verts.size(); i++) {
		Vec3f v0 = deformed_face_verts[i];
		Vec3f v1 = deformed_face_verts[(i+1)%deformed_face_verts.size()];
		line (v0, v1, image1, red);
	}

	// Drawing the deformed model (central_projection ()):
	deformed_face_verts = m2vs(VP*(vs2m(central_projection (face_verts, Vec3f(5,0,0)))));
	for (unsigned int i = 0; i<deformed_face_verts.size(); i++) {
		Vec3f v0 = deformed_face_verts[i];
		Vec3f v1 = deformed_face_verts[(i+1)%deformed_face_verts.size()];
		line (v0, v1, image2, blue);
	}

	image1.flip_vertically();	
	image2.flip_vertically();	
	image1.write_tga_file("projection.tga");
	image2.write_tga_file("central_projection.tga");
	
	return 0;
}

Matrix v2m (Vec3f v) {
	Matrix m (4,1);
	for (int i = 0; i<3; i++) m[i][0] = v[i];
	m[3][0] = 1;
	return m;
}

Vec3f m2v (Matrix m) {
	return Vec3f (m[0][0]/m[3][0], m[1][0]/m[3][0], m[2][0]/m[3][0]);
}

vector<Vec3f> m2vs (Matrix m) {
	vector<Vec3f> vs;
	for (int i = 0; i < m.ncols(); i++)
		vs.push_back (Vec3f (m[0][i]/m[3][i], m[1][i]/m[3][i], m[2][i]/m[3][i]));
	return vs;
}

Matrix vs2m (vector<Vec3f> vs) {
	Matrix m (4, vs.size ());
	for (unsigned int j = 0; j < vs.size (); j++) {
		for (int i = 0; i<3; i++) m[i][j] = vs[j][i];
		m[3][j] = 1;
	}
	return m;
}

Matrix scale (float x_scale, float y_scale, float z_scale) {
	Matrix scale_matrix = Matrix::identity (4);
	scale_matrix[0][0] = x_scale;
	scale_matrix[1][1] = y_scale;
	scale_matrix[2][2] = z_scale;
	return scale_matrix;
}

Matrix shear_x (float shear) {
	Matrix shear_matrix = Matrix::identity (4);
	shear_matrix[0][1] = shear;
	return shear_matrix;
}

Matrix shear_y (float shear) {
	Matrix shear_matrix = Matrix::identity (4);
	shear_matrix[1][0] = shear;
	return shear_matrix;
}

Matrix rotate (float alpha) {
	Matrix rotate_matrix = Matrix::identity (4);
	rotate_matrix[0][0] = cos (alpha * PI / 180.0);
	rotate_matrix[0][1] = -sin (alpha * PI / 180.0);
	rotate_matrix[1][0] = sin (alpha * PI / 180.0);
	rotate_matrix[1][1] = cos (alpha * PI / 180.0);
	return rotate_matrix;
}

Matrix translate (float x_offset, float y_offset, float z_offset) {
	Matrix translate_matrix = Matrix::identity (4);
	translate_matrix[0][3] = x_offset;
	translate_matrix[1][3] = y_offset;
	translate_matrix[2][3] = z_offset;
	return translate_matrix;
}

Matrix viewport (int x_offset, int y_offset, int window_width, int window_height) {
	return translate (window_width/2+x_offset,window_height/2+y_offset, depth/2.) * scale (window_width/2., window_height/2., depth/2.);
}

Matrix projection (float p) {
	Matrix project_matrix = Matrix::identity (4);
	project_matrix[3][0] = p;
	return project_matrix;
}

vector<Vec3f> central_projection (vector<Vec3f> pts, Vec3f camera) {
	float a[pts.size ()]; 
	float b[pts.size ()];
	// Finding ntersections with the screen line: 
	for (unsigned int i = 0; i < pts.size (); i++) {
		a[i] = (pts[i].y-camera.y)/(pts[i].x-camera.x);
		b[i] = pts[i].y - (a[i] * pts[i].x);
	}
	for (unsigned int i = 0; i < pts.size (); i++)
		pts[i].y = b[i];
	
	// Using: x' = x / (1 - z/c):
	// We are working in 2D: z is x.
	for (unsigned int i = 0; i < pts.size (); i++)
		pts[i].x = pts[i].x / (1. - pts[i].x/camera.x);
	
	return pts;
}

void rasterize (Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color, int *ybuffer) {
	if (p0.x > p1.x)
		swap (p0, p1);

	int Dx = (p1.x - p0.x), Dy = (p1.y - p0.y);
	for (int x = p0.x; x <= p1.x; x++) {
		float t = (x - p0.x) / (float) Dx;
		int y = p0.y + Dy * t;
		if (ybuffer[x] < y) {
			ybuffer[x] = y;
			for (int i = 0; i <16; i++)
				image.set (x, i, color);
		}
	}
}

Vec3f barycentric (Vec3f *pts, Vec3f P) {
	Vec3f A = pts[0], B = pts[1], C = pts[2];

	float D = (A.x-C.x)*(B.y-C.y) - (B.x-C.x)*(A.y-C.y);
	if (D == 0) return Vec3f (-1,-1,-1); // Getting rid of degenerate triangles.

	float alpha = ((B.y-C.y)*(P.x-C.x) + (C.x-B.x)*(P.y-C.y)) / D;
	float beta = ((C.y-A.y)*(P.x-C.x) + (A.x-C.x)*(P.y-C.y)) / D;

	return Vec3f (alpha, beta, (1. - alpha - beta));
}

void triangle (Vec3f  *pts, float *zbuffer, TGAImage &image, float intensity, Vec3f *texture_coords) {
	Vec2f min_max [2]; // [min_x, min_y, max_x, max_y]
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			min_max[i][j] =  (i==0)? fmin (pts[0][j], fmin (pts[1][j], pts[2][j])) : fmax (pts[0][j], fmax (pts[1][j], pts[2][j]));
	Vec2f bboxmin (fmax (0.f, min_max[0][0]), fmax (0.f, min_max[0][1]));
	Vec2f bboxmax (fmin (float(image.get_width()-1), min_max[1][0]), fmin (float(image.get_height()-1), min_max[1][1]));

	Vec3f P;
	for (P.x = roundf (bboxmin.x); P.x <= roundf (bboxmax.x); P.x++)
		for (P.y = roundf (bboxmin.y); P.y <= roundf (bboxmax.y); P.y++) {
			Vec3f bc_P = barycentric (pts, P);
			if (bc_P.x >= 0 &&  bc_P.y >= 0 && bc_P.z >= 0) {
				P.z = 0;
				for (int i=0; i<3; i++) P.z += pts[i].z*bc_P[i];
				if (zbuffer[int(P.x+P.y*width)] < P.z) {
					zbuffer[int(P.x+P.y*width)] = P.z;
					TGAColor color = (texture_coords == NULL)? TGAColor(255, 255, 255, 255) : model->getTextureColor (texture_coords, bc_P);
					for (int i=0; i<3; i++) color[i] = color[i] * intensity;
					image.set (P.x, P.y, color);
				}
			}
		}
}

void line(Vec3i p0, Vec3i p1, TGAImage &image, TGAColor color) {
	line(Vec2i (p0.x, p0.y), Vec2i (p1.x, p1.y), image, color);
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