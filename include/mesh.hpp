#ifndef MESH_H
#define MESH_H

#include <vector>
#include "object3d.hpp"
#include "triangle.hpp"
#include "Vector2f.h"
#include "Vector3f.h"


class Mesh : public Object3D {

private:

    // Normal can be used for light estimation
    void computeNormal();

public:

    Mesh(const char *filename, Material *m);

    struct TriangleIndex {
        int x[3]{};
        TriangleIndex() {
            x[0] = 0; x[1] = 0; x[2] = 0;
        }
        int &operator[](const int i) { return x[i]; }
        // By Computer Graphics convention, counterclockwise winding is front face
    };

    std::vector<Vector3f> vertices;
    std::vector<TriangleIndex> triangles;
    std::vector<Vector3f> normals;
    bool intersect(const Ray &r, Hit &h, float tmin) override;
};

#endif
