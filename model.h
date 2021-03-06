#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>

#include "tgaimage.h"
#include "geometry.h"

class Model {
private:
	const char *model_filename_;
	const char *texture_filename_;
	const char *normalMap_filename_;
	const char *specularMap_filename_;
	TGAImage *texture_;
	TGAImage *normalMap_;
	TGAImage *specularMap_;

	int facesFormat_;
	std::vector<Vec3f> verts_;
	std::vector<std::vector<Vec3i> > faces_;
	std::vector<Vec3f> vts_;
	std::vector<Vec3f> vns_;

	float m_minX;
	float m_maxX;
	float m_minY;
	float m_maxY;

private:
	void Init ();
	void loadTexture ();
	void loadNormalMap ();
	void loadSpecularMap ();

	void minMaxXY ();
	float minX ();
	float maxX ();
	float minY ();
	float maxY ();
	void normalize ();

	int nSides (std::string line);
	void facesFormat (std::string line);

public:
	Model (const char *filename);
	Model (const char *model_filename, const char *texture_filename);
	Model (const char *model_filename, const char *texture_filename, const char *normalMap_filename);
	Model (const char *model_filename, const char *normalMap_filename, bool no_texture);
	Model (const char *model_filename, const char *texture_filename, const char *normalMap_filename, const char *specularMap_filename);

	~Model ();

	TGAColor getTextureColor (Vec3f *texture_coords, Vec3f P);
	bool has_texture ();

	Vec3f normal (int iface, int nthvert);
	Vec3f normal_from_map (Vec3f *nmap_coords, Vec3f P);
	bool has_normal_map ();
	
	float specular (Vec3f *specular_coords, Vec3f P);
	bool has_specular_map ();

	int nverts ();
	int nfaces ();
	int nvts ();
	std::vector<Vec3i> face (int idx);
	Vec3f vert (int i);
	Vec3f vert (int iface, int nthvert);
	Vec3f vt (int idx);
	Vec3f vn (int idx);
	Vec3f vt (int iface, int nthvert);
};

#endif //__MODEL_H__