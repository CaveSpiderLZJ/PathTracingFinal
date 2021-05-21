#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <vecmath.h>
#include "object3d.hpp"

// transforms a 3D point using a matrix, returning a 3D point
static Vector3f transformPoint(const Matrix4f &mat, const Vector3f &point) {
    return (mat * Vector4f(point, 1)).xyz();
}

// transform a 3D directino using a matrix, returning a direction
static Vector3f transformDirection(const Matrix4f &mat, const Vector3f &dir) {
    return (mat * Vector4f(dir, 0)).xyz();
}

// TODO: implement this class so that the intersect function first transforms the ray
class Transform : public Object3D {

protected:

    Object3D *obj; //un-transformed object
    Matrix4f transform;

public:
    Transform() {}

    Transform(const Matrix4f &m, Object3D *_obj) : obj(_obj) {
        transform = m.inverse();
    }

    ~Transform() {
    }

    virtual bool intersect(const Ray &ray, Hit &hit, float tmin) {
        Vector3f trSource = transformPoint(transform, ray.getOrigin());
        Vector3f trDirection = transformDirection(transform, ray.getDirection());
        Ray tr(trSource, trDirection.normalized());
        bool isIntersected = obj->intersect(tr, hit, tmin);
        if(!isIntersected) return false;
        Vector3f transformedN = transformDirection(transform.transposed(), hit.getNormal());
        hit.set(hit.getT() / trDirection.length(), hit.getMaterial(), transformedN);
        return true;
    }
};

#endif //TRANSFORM_H
