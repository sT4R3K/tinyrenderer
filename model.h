#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	int m_facesFormat;
	std::vector<Vec3f> verts_;
	std::vector<std::vector<int> > faces_;
	std::vector<Vec3f> vts_;

	float m_minX;
	float m_maxX;
	float m_minY;
	float m_maxY;

	

public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	int nvts();
	Vec3f vert(int i);
	std::vector<int> face(int idx);
	Vec3f vt(int idx);

	void minMaxXY ();
	float minX ();
	float maxX ();
	float minY ();
	float maxY ();
	void normalize ();

	int nSides (std::string line);
	void facesFormat (std::string line);
};

#endif //__MODEL_H__