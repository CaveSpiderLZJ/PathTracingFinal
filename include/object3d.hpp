#ifndef OBJECT3D_H
#define OBJECT3D_H

#include "ray.hpp"
#include "hit.hpp"
#include "material.hpp"
#include <cmath>

// Base class for all 3d entities.
class Object3D {

protected:

    Material *material;

public:

    Object3D(): material(nullptr) {}

    virtual ~Object3D() = default;

    explicit Object3D(Material *_material) {
        this->material = _material;
    }

    Material* getMaterial(){
        return this->material;
    }

    // Intersect Ray with this object. If hit, store information in hit structure.
    virtual bool intersect(const Ray &r, Hit &h, float tmin) = 0;

    // static method
    static float disPoint2Point(const Vector3f& point1, const Vector3f& point2){
        float dx = point1.x() - point2.x();
        float dy = point1.y() - point2.y();
        float dz = point1.z() - point2.z();
        return sqrt(dx * dx + dy * dy + dz * dz);
    }

    static float disPoint2Ray(const Vector3f& point, const Ray& ray, bool& isAhead){
        Vector3f origin = ray.getOrigin();
        Vector3f direction = ray.getDirection().normalized();
        float t = Vector3f::dot(direction, (point - origin));
        isAhead = (t >= 0);
        return disPoint2Point(point, origin + (t*direction));
    }
};

#endif

