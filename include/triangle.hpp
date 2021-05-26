#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "object3d.hpp"
#include "plane.hpp"
#include "hit.hpp"
#include <vecmath.h>
#include <cmath>
#include <iostream>

// TODO: implement this class and add more fields as necessary,
class Triangle: public Object3D {

public:

    Vector3f normal;
	Vector3f vertices[3];

	Triangle() = delete;

    // a b c are three vertex positions of the triangle
	Triangle(const Vector3f& a, const Vector3f& b, const Vector3f& c, Material* m) : Object3D(m) {
        vertices[0] = a;
        vertices[1] = b;
        vertices[2] = c;
        normal = Vector3f::cross(b - a, c - a).normalized();
        return;
	}

	bool intersect(const Ray& ray, Hit& hit, float tmin) override {
        Vector3f E1 = vertices[0] - vertices[1];
        Vector3f E2 = vertices[0] - vertices[2];
        Vector3f S = vertices[0] - ray.getOrigin();
        Vector3f Rd = ray.getDirection().normalized();
        float divisor = Matrix3f(Rd, E1, E2).determinant();
        if(divisor == 0){
            return false;
        }
        // t, beta, gamma
        Vector3f res = Vector3f(Matrix3f(S, E1, E2).determinant(),
                                Matrix3f(Rd, S, E2).determinant(),
                                Matrix3f(Rd, E1, S).determinant()) / divisor;
        if(res[0] < tmin || res[1] < 0 || res[1] > 1 || res[2] < 0 || res[2] > 1 || res[1] + res[2] > 1)
            return false;
        Vector3f n;
        if(Vector3f::dot(Rd, normal) > 0) n = -1 * normal;
        else n = normal;
        hit.set(res[0], material, n);
        return true;
	}
};

#endif //TRIANGLE_H
