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
    Vector3f w1, w2;   // 与normal共同构成正交基向量 
    float d;

public:
    Plane() {
        // xy plane
        d = 0;
        normal = Vector3f(0, 0, 1);
        w1 = Vector3f(1, 0, 0);
        w2 = Vector3f(0, 1, 0);
        return;
    }

    Plane(const Vector3f &_normal, float _d, Material *_m) : Object3D(_m) {
        normal = _normal.normalized();
        w1 = (Vector3f::cross(normal[0] > 0.1f ? Vector3f(0, 1, 0) : Vector3f(1, 0, 0), normal)).normalized();
        w2 = Vector3f::cross(normal, w1);
        d = _d;
        return;
    }

    ~Plane() override = default;

    bool intersect(const Ray &ray, Hit &hit, float tmin, float& u, float& v) override {
        float dotND = Vector3f::dot(normal, ray.direction);
        if(dotND == 0) return false;
        float t = (d - Vector3f::dot(normal, ray.origin)) / dotND;
        if(t < tmin) return false;
        Vector3f n;
        if(Vector3f::dot(ray.direction, normal) > 0.0f) n = -1 * normal;
        else n = normal;
        hit.set(t, material, n);
        Vector3f intersect = ray.origin + ray.direction * t - (d * normal); // 平移到以原点为中心的交点
        u = Vector3f::dot(w2, intersect);
        v = Vector3f::dot(w1, intersect);
        return true;
    }
};

#endif //PLANE_H
		

