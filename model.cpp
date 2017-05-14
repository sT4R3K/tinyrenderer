#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include "model.h"

Model::Model(const char *filename) : m_facesFormat(0), verts_(), faces_(), vts_() {
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
            // Check if the face is a triangle:
            if (false /*nSides (line) != 3*/) {
                std::cerr << "The object in \"" << filename << "\" contains at least a face that is not a triangle." << std::endl;
                std::cerr << "Aborted!" << std::endl;
                exit (0); // The dirty way XD
            }

            std::vector<int> f;
            std::vector<int> f_vt;
            int itrash, idx, vt;
            iss >> trash;

            facesFormat (line);
            switch (m_facesFormat) {
                case 1: // f v v v
                    while (iss >> idx) {
                        idx--; // in wavefront obj all indices start at 1, not zero
                        f.push_back(idx);
                    }
                    break;
                case 2: // f v/vt v/vt v/vt
                    while (iss >> idx >> trash >> vt) {
                        idx--;
                        f.push_back(idx);
                        vt--;
                        f_vt.push_back (vt);
                    }
                    break;
                case 5: // f v/vt/vn v/vt/vn v/vt/vn v/vt/vn
                case 6: // f v/vt/vn v/vt/vn v/vt/vn v/vt/vn v/vt/vn
                case 3: // f v/vt/vn v/vt/vn v/vt/vn
                    while (iss >> idx >> trash >> vt >> trash >> itrash) {
                        idx--;
                        f.push_back(idx);
                        vt--;
                        f_vt.push_back (vt);
                    }
                    break;
                case 4: // f v//vn v//vn v//vn
                    while (iss >> idx >> trash >> trash >> itrash) {
                        idx--;
                        f.push_back(idx);
                    }
                    break;
            }
            while (f_vt.size () > 0) {
                f.push_back (f_vt.back ());
                f_vt.pop_back ();
            }
            faces_.push_back(f);
        } else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            Vec3f vt;
            for (int i=0;i<3;i++) iss >> vt.raw[i];
            vts_.push_back(vt);
        }
    }

    // Normaliser l'objet (coordonnées entre -1 et 1):
    //minMaxXY ();
    //normalize ();

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

int Model::nvts () {
    return (int) vts_.size ();
}
std::vector<int> Model::face(int idx) {
    return faces_[idx];
}

Vec3f Model::vert(int i) {
    return verts_[i];
}

Vec3f Model::vt (int idx) {
    return vts_[idx];
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

int Model::nSides (std::string line) {
    int n = 0;
    for (unsigned int i = 0; i < line.length (); i++)
        for (n++; i < line.length () && line.at (i) != ' '; i++); 
    
    return n-1; // Ignore the 'f'.
}

/* 
    We sould be able to parse all the following faces representations:
        1: f v v v
        2: f v/vt v/vt v/vt
        3: f v/vt/vn v/vt/vn v/vt/vn
        4: f v//vn v//vn v//vn
        5: f v/vt/vn v/vt/vn v/vt/vn v/vt/vn
        6: f v/vt/vn v/vt/vn v/vt/vn v/vt/vn v/vt/vn
*/
void Model::facesFormat (std::string line) {
    // This function will be executed only one time during the lifetime of the object:
    if (m_facesFormat != 0)
        return;

    int n = 0;
    // Counting the '/' s:
    for (unsigned int i = 0; i < line.length (); i++)
        if (line.at (i) == '/')
            n++;

    switch (n) {
        case 0:
            m_facesFormat = 1;
            return;
        case 3:
            m_facesFormat = 2;
            return;
        case 6:
            break;
        case 8:
            m_facesFormat = 5;
            break;
        case 10:
            m_facesFormat = 6;
            break;
        default:
            std::cerr << "Unknown faces representation!" << std::endl;
            exit (0); // No time for a fancy way >__<
    }

    // We search for double slashes:
    if (line.find ("//") == std::string::npos)
        m_facesFormat = 3;
    else
        m_facesFormat = 4;
}