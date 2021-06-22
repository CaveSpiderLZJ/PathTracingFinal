#ifndef RAY_H
#define RAY_H

#include <cassert>
#include <iostream>
#include <Vector3f.h>


// Ray class mostly copied from Peter Shirley and Keith Morley
class Ray {

public:

    Vector3f origin;
    Vector3f direction;
    Vector3f pastColor;     // 记录光线之前叠乘的颜色
    int depth;

    Ray() = delete;

    Ray(const Vector3f& _origin, const Vector3f& _direction,
        const Vector3f& _pastColor = Vector3f(1, 1, 1),
        int _depth = 0) {
        origin = _origin;
        direction = _direction.normalized();
        pastColor = _pastColor;
        depth = _depth;
    }

    Ray(const Ray &ray) {
        origin = ray.origin;
        direction = ray.direction.normalized();
        pastColor = ray.pastColor;
    }

    const Vector3f &getOrigin() const {
        return origin;
    }

    void setOrigin(const Vector3f& _origin){
        origin = _origin;
        return;
    }

    const Vector3f &getDirection() const {
        return direction;
    }

    void setDirection(const Vector3f& _direction){
        direction = _direction.normalized();
        return;
    }

    Vector3f pointAtParameter(float t) const {
        return origin + direction * t;
    }
};

inline std::ostream &operator<<(std::ostream &os, const Ray &ray) {
    os << "Ray <" << ray.getOrigin() << ", " << ray.getDirection()
        << ", " << ">";
    return os;
}

#endif // RAY_H
