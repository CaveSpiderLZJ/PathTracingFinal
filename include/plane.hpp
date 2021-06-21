#ifndef PLANE_H
#define PLANE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

// TODO: Implement Plane representing an infinite plane
// function: ax+by+cz=d
// choose your representation , add more fields and fill in the functions

class Plane : public Object3D {

protected:

    Vector3f normal;
    float d;

public:
    Plane() {
        // xy plane
        d = 0;
        normal = Vector3f(0, 0, 1);
        return;
    }

    Plane(const Vector3f &_normal, float _d, Material *_m) : Object3D(_m) {
        normal = _normal.normalized();
        d = _d;
        return;
    }

    ~Plane() override = default;

    bool intersect(const Ray &ray, Hit &hit, float tmin, float& u, float& v) override {
        float dotND = Vector3f::dot(normal, ray.getDirection().normalized());
        if( dotND == 0){
            return false;
        }
        float t = (d - Vector3f::dot(normal, ray.getOrigin())) / dotND;
        if(t < tmin){
            return false;
        }
        Vector3f n;
        if(Vector3f::dot(ray.getDirection(), normal) > 0) n = -1 * normal;
        else n = normal;
        hit.set(t, material, n);
        return true;
    }
};

#endif //PLANE_H
		

