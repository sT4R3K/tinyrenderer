#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) : verts_(), faces_() {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v.raw[i];
            verts_.push_back(v);
        } else if (!line.compare(0, 2, "f ")) {
            std::vector<int> f;
            int itrash, idx;
            iss >> trash;
            while (iss >> idx >> trash >> itrash >> trash >> itrash) {
                idx--; // in wavefront obj all indices start at 1, not zero
                f.push_back(idx);
            }
            faces_.push_back(f);
        }
    }

    // Normaliser l'objet (coordonnées entre -1 et 1):
    minMaxXY ();
    normalize ();
    minMaxXY ();

    std::cerr << "# v# " << verts_.size() << " f# "  << faces_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

std::vector<int> Model::face(int idx) {
    return faces_[idx];
}

Vec3f Model::vert(int i) {
    return verts_[i];
}

void Model::minMaxXY () {
    m_minX = verts_[0].x;
    m_maxX = verts_[0].x;
    m_minY = verts_[0].y;
    m_maxY = verts_[0].y;

    for (int i = 1; i < nverts (); i++) {
        if (m_minX > verts_[i].x)
            m_minX = verts_[i].x;
        if (m_maxX < verts_[i].x)
            m_maxX = verts_[i].x;
        if (m_minY > verts_[i].y)
            m_minY = verts_[i].y;
        if (m_maxY < verts_[i].y)
            m_maxY = verts_[i].y;
    }
}

void Model::normalize () {
    float width = std::abs (maxX () - minX ());
    float height = std::abs (maxY () - minY ());
    float maxWidthHeight = ((width>=height)? width : height);

    // Centering coordinates around the origin:
    float xStep = (maxX () + minX ()) / 2.;
    float yStep = (maxY () + minY ()) / 2.;
    for (int i = 0; i < nverts (); ++i) {
        verts_[i].x = verts_[i].x - xStep;
        verts_[i].y = verts_[i].y - yStep;
    }

    // Reducing coordinates to the range [-1;1]
    for (int i = 0; i < nverts (); ++i) {
        verts_[i].x = 2.*verts_[i].x/maxWidthHeight;
        verts_[i].y = 2.*verts_[i].y/maxWidthHeight;
    }
}

float Model::minX () {
    return m_minX;
}
float Model::maxX () {
    return m_maxX;
}
float Model::minY () {
    return m_minY;
}
float Model::maxY () {
    return m_maxY;
}

