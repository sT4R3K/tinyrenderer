#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>

#include "model.h"

Model::Model(const char *filename) : facesFormat_(0), verts_(), faces_(), vts_(), vns_() {
    model_filename_ = filename;
    texture_filename_ = NULL;
    texture_ = NULL;
    normalMap_ = NULL;
    specularMap_ = NULL;
    std::cout << "Loading " << filename << "..." << std::endl;
    Init ();
    std::cout << "OK" << std::endl;
}

Model::Model(const char *model_filename, const char *texture_filename) : facesFormat_(0), verts_(), faces_(), vts_(), vns_() {
    model_filename_ = model_filename;
    texture_filename_ = texture_filename;
    normalMap_ = NULL;
    specularMap_ = NULL;
    std::cout << "Loading " << model_filename << "..." << std::endl;
    Init ();
    std::cout << "OK" << std::endl;
    std::cout << "Loading " << texture_filename << "..." << std::endl;
    loadTexture ();
    std::cout << "OK" << std::endl;
}

Model::Model (const char *model_filename, const char *texture_filename, const char *normalMap_filename) : facesFormat_(0), verts_(), faces_(), vts_(), vns_() {
    model_filename_ = model_filename;
    texture_filename_ = texture_filename;
    normalMap_filename_ = normalMap_filename;
    specularMap_ = NULL;
    std::cout << "Loading " << model_filename << "..." << std::endl;
    Init ();
    std::cout << "OK" << std::endl;
    std::cout << "Loading " << texture_filename << "..." << std::endl;
    loadTexture ();
    std::cout << "OK" << std::endl;
    std::cout << "Loading " << normalMap_filename << "..." << std::endl;
    loadNormalMap ();
    std::cout << "OK" << std::endl;
}

Model::Model (const char *model_filename, const char *normalMap_filename, bool no_texture) : facesFormat_(0), verts_(), faces_(), vts_(), vns_() {
    model_filename_ = model_filename;
    normalMap_filename_ = normalMap_filename;
    texture_ = NULL;
    specularMap_ = NULL;
    std::cout << "Loading " << model_filename << "..." << std::endl;
    Init ();
    std::cout << "OK" << std::endl;
    std::cout << "Loading " << normalMap_filename << "..." << std::endl;
    loadNormalMap ();
    std::cout << "OK" << std::endl;
}

Model::Model (const char *model_filename, const char *texture_filename, const char *normalMap_filename, const char *specularMap_filename) {
    model_filename_ = model_filename;
    texture_filename_ = texture_filename;
    normalMap_filename_ = normalMap_filename;
    specularMap_filename_ =specularMap_filename;
    std::cout << "Loading " << model_filename << "..." << std::endl;
    Init ();
    std::cout << "OK" << std::endl;
    std::cout << "Loading " << texture_filename << "..." << std::endl;
    loadTexture ();
    std::cout << "OK" << std::endl;
    std::cout << "Loading " << normalMap_filename << "..." << std::endl;
    loadNormalMap ();
    std::cout << "OK" << std::endl;
    std::cout << "Loading " << specularMap_filename << "..." << std::endl;
    loadSpecularMap ();
    std::cout << "OK" << std::endl;
}


void Model::Init () {
    std::ifstream in;
    in.open (model_filename_, std::ifstream::in);
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
            /*
            // Check if the face is a triangle:
            if (nSides (line) != 3) {
                std::cerr << "The object in \"" << model_filename_ << "\" contains at least a face that is not a triangle." << std::endl;
                std::cerr << "Aborted!" << std::endl;
                exit (0); // The dirty way XD
            }
            //*/
            std::vector<Vec3i> f;
            int idx, vt, vn;
            iss >> trash;

            facesFormat (line);
            switch (facesFormat_) {
                case 1: // f v v v
                    while (iss >> idx) {
                        idx--; // in wavefront obj all indices start at 1, not zero
                        f.push_back(Vec3i (idx, 0, 0));
                    }
                    break;
                case 2: // f v/vt v/vt v/vt
                    while (iss >> idx >> trash >> vt) {
                        idx--;
                        vt--;
                        f.push_back(Vec3i (idx, vt, 0));
                    }
                    break;
                case 5: // f v/vt/vn v/vt/vn v/vt/vn v/vt/vn
                case 6: // f v/vt/vn v/vt/vn v/vt/vn v/vt/vn v/vt/vn
                case 3: // f v/vt/vn v/vt/vn v/vt/vn
                    while (iss >> idx >> trash >> vt >> trash >> vn) {
                        idx--;
                        vt--;
                        vn--;
                        f.push_back(Vec3i (idx, vt, vn));
                    }
                    break;
                case 4: // f v//vn v//vn v//vn
                    while (iss >> idx >> trash >> trash >> vn) {
                        idx--;
                        vn--;
                        f.push_back(Vec3i (idx, 0, vn));
                    }
                    break;
            }
            faces_.push_back (f);
        } else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            Vec3f vt;
            for (int i=0;i<3;i++) iss >> vt.raw[i];
            vts_.push_back(vt);
        } else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            Vec3f vn;
            for (int i = 0; i < 3; i++) iss >> vn.raw[i];
            vns_.push_back (vn);
        }
    }

    // Normaliser l'objet (coordonnées entre -1 et 1):
    minMaxXY ();
    normalize ();

    std::cerr << "# v# " << verts_.size() << " f# "  << faces_.size() << std::endl;
}


Model::~Model() {
}

void Model::loadTexture () {
    texture_ = new TGAImage ();
    texture_->read_tga_file (texture_filename_);
    texture_->flip_vertically ();
}

void Model::loadNormalMap () {
    normalMap_ = new TGAImage ();
    normalMap_->read_tga_file (normalMap_filename_);
    normalMap_->flip_vertically ();
}

void Model::loadSpecularMap () {
    specularMap_ = new TGAImage ();
    specularMap_->read_tga_file (specularMap_filename_);
    specularMap_->flip_vertically ();
}

TGAColor Model::getTextureColor (Vec3f *texture_coords, Vec3f P) {
    Vec2f T; // T : interpolated P inside the triangle *texture_coords.
    for (int i=0; i<3; i++) T.x += texture_coords[i][0]*P[i];
    for (int i=0; i<3; i++) T.y += texture_coords[i][1]*P[i];

    return texture_->get (roundf(T.x * texture_->get_width ()), roundf(T.y * texture_->get_height ()));
}

bool Model::has_texture () {
    return (texture_ != NULL);
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
std::vector<Vec3i> Model::face(int idx) {
    return faces_[idx];
}

Vec3f Model::vert(int i) {
    return verts_[i];
}

Vec3f Model::vert (int iface, int nthvert) {
    return vert ((face (iface))[nthvert][0]);
}

Vec3f Model::vt (int idx) {
    return vts_[idx];
}

Vec3f Model::vn (int idx) {
    return vns_[idx];
}

Vec3f Model::normal (int iface, int nthvert) {
    return vn ((face (iface))[nthvert][2]);
}

Vec3f Model::normal_from_map (Vec3f *nmap_coords, Vec3f P) {
    Vec2f T; // T : interpolated P inside the triangle *nmap_coords.
    for (int i=0; i<3; i++) T.x += nmap_coords[i][0]*P[i];
    for (int i=0; i<3; i++) T.y += nmap_coords[i][1]*P[i];

    TGAColor color = normalMap_->get (roundf(T.x * normalMap_->get_width ()), roundf(T.y * normalMap_->get_height ()));
    Vec3f res;
    for (int i = 0; i < 3; i++)
        res[2-i] = (float)color[i]/255.f*2.f - 1.f;
    return res;
}

bool Model::has_normal_map () {
    return (normalMap_ != NULL);
}

Vec3f Model::vt (int iface, int nthvert) {
    return vt ((face (iface))[nthvert][1]);
}

float Model::specular (Vec3f *specular_coords, Vec3f P) {
    Vec2f T; // T : interpolated P inside the triangle *specular_coords.
    for (int i=0; i<3; i++) T.x += specular_coords[i][0]*P[i];
    for (int i=0; i<3; i++) T.y += specular_coords[i][1]*P[i];

    return specularMap_->get(roundf(T.x * specularMap_->get_width ()), roundf(T.y * specularMap_->get_height ()))[0]/1.f;
}

bool Model::has_specular_map () {
    return (texture_ != NULL);
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
    if (facesFormat_ != 0)
        return;

    int n = 0;
    // Counting the '/' s:
    for (unsigned int i = 0; i < line.length (); i++)
        if (line.at (i) == '/')
            n++;

    switch (n) {
        case 0:
            facesFormat_ = 1;
            return;
        case 3:
            facesFormat_ = 2;
            return;
        case 6:
            break;
        case 8:
            facesFormat_ = 5;
            break;
        case 10:
            facesFormat_ = 6;
            break;
        default:
            std::cerr << "Unknown faces representation!" << std::endl;
            exit (0); // No time for a fancy way >__<
    }

    // We search for double slashes:
    if (line.find ("//") == std::string::npos)
        facesFormat_ = 3;
    else
        facesFormat_ = 4;
}