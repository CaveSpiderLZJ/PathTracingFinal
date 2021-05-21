#include "mesh.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <utility>
#include <sstream>

bool Mesh::intersect(const Ray &ray, Hit &hit, float tmin) {
    // Optional: Change this brute force method into a faster one.
    float t = 1e38;
    Vector3f n;
    Material* m;
    bool isIntersected = false;
    for (int triId = 0; triId < (int) triangles.size(); ++triId) {
        TriangleIndex& triIndex = triangles[triId];
        Triangle triangle(vertices[triIndex[0]],
                          vertices[triIndex[1]], vertices[triIndex[2]], material);
        //triangle.normal = n[triId];
        if(triangle.intersect(ray, hit, tmin) && hit.getT() < t){
            isIntersected = true;
            t = hit.getT();
            n = hit.getNormal();
            m = hit.getMaterial();
        }
    }
    if(!isIntersected) return false;
    hit.set(t, m, n);
    return true;
}

Mesh::Mesh(const char *filename, Material *material) : Object3D(material) {

    // Optional: Use tiny obj loader to replace this simple one.
    std::ifstream f;
    f.open(filename);
    if (!f.is_open()) {
        std::cout << "Cannot open " << filename << "\n";
        return;
    }
    std::string line;
    std::string vTok("v");
    std::string fTok("f");
    std::string texTok("vt");
    char bslash = '/', space = ' ';
    std::string tok;
    int texID;
    while (true) {
        std::getline(f, line);
        if (f.eof()) {
            break;
        }
        if (line.size() < 3) {
            continue;
        }
        if (line.at(0) == '#') {
            continue;
        }
        std::stringstream ss(line);
        ss >> tok;
        if (tok == vTok) {
            Vector3f vec;
            ss >> vec[0] >> vec[1] >> vec[2];
            vertices.push_back(vec);
        } else if (tok == fTok) {
            if (line.find(bslash) != std::string::npos) {
                std::replace(line.begin(), line.end(), bslash, space);
                std::stringstream facess(line);
                TriangleIndex trig;
                facess >> tok;
                for (int ii = 0; ii < 3; ii++) {
                    facess >> trig[ii] >> texID;
                    trig[ii]--;
                }
                triangles.push_back(trig);
            } else {
                TriangleIndex trig;
                for (int ii = 0; ii < 3; ii++) {
                    ss >> trig[ii];
                    trig[ii]--;
                }
                triangles.push_back(trig);
            }
        } else if (tok == texTok) {
            Vector2f texcoord;
            ss >> texcoord[0];
            ss >> texcoord[1];
        }
    }
    f.close();
}

void Mesh::computeNormal() {
    normals.resize(triangles.size());
    for (int triId = 0; triId < (int) triangles.size(); ++triId) {
        TriangleIndex& triIndex = triangles[triId];
        Vector3f a = vertices[triIndex[1]] - vertices[triIndex[0]];
        Vector3f b = vertices[triIndex[2]] - vertices[triIndex[0]];
        b = Vector3f::cross(a, b);
        normals[triId] = b / b.length();
    }
}
