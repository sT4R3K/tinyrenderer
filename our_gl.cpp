#include "our_gl.h"

Matrix v2m (Vec3f v) {
	Matrix m (4,1);
	for (int i = 0; i<3; i++) m[i][0] = v[i];
	m[3][0] = 1;
	return m;
}

Matrix v2m (Vec4f v) {
	Matrix m (4,1);
	for (int i = 0; i<4; i++) m[i][0] = v[i];
	return m;
}

Vec3f m2v (Matrix m) {
	return Vec3f (m[0][0]/m[3][0], m[1][0]/m[3][0], m[2][0]/m[3][0]);
}

Vec4f m2v4 (Matrix m) {
	return Vec4f (m[0][0]/m[3][0], m[1][0]/m[3][0], m[2][0]/m[3][0], m[3][0]/m[3][0]);
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

Matrix projection (float c) {
	Matrix project_matrix = Matrix::identity (4);
	project_matrix[3][2] = c;
	return project_matrix;
}

Matrix lookat(Vec3f eye, Vec3f center, Vec3f up) {
    Vec3f z = (eye-center).normalize();
    Vec3f x = cross(up,z).normalize();
    Vec3f y = cross(z,x).normalize();
    Matrix Minv = Matrix::identity(4);
    Matrix Tr   = Matrix::identity(4);
    for (int i=0; i<3; i++) {
        Minv[0][i] = x[i];
        Minv[1][i] = y[i];
        Minv[2][i] = z[i];
        Tr[i][3] = -center[i];
    }
    return Minv*Tr;
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

Vec3f barycentric (Vec4f *pts, Vec3f P) {
	Vec4f A = pts[0], B = pts[1], C = pts[2];

	float D = (A.x-C.x)*(B.y-C.y) - (B.x-C.x)*(A.y-C.y);
	if (D == 0) return Vec3f (-1,-1,-1); // Getting rid of degenerate triangles.

	float alpha = ((B.y-C.y)*(P.x-C.x) + (C.x-B.x)*(P.y-C.y)) / D;
	float beta = ((C.y-A.y)*(P.x-C.x) + (A.x-C.x)*(P.y-C.y)) / D;

	return Vec3f (alpha, beta, (1. - alpha - beta));
}

void triangle (vector<Vec4f> pts, IShader *shader, float *zbuffer, TGAImage &image, Vec3f *texture_coords){
	Vec4f points[3];
	for (int i = 0; i < 3; ++i)
		points[i] = pts.at (i);
	triangle (points, shader, zbuffer, image, texture_coords);
}

void triangle (Vec4f  *pts, IShader *shader, float *zbuffer, TGAImage &image, Vec3f *texture_coords) {
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
					TGAColor color;
					if (!shader->fragment (bc_P, color)) {
						zbuffer[int(P.x+P.y*width)] = P.z;
						image.set (P.x, P.y, color);
					}
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
