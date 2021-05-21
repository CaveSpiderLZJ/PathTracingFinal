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
    float intensity;
    bool isOutside;

    Ray() = delete;

    Ray(const Vector3f& _origin, const Vector3f& _direction,
        const float& _intensity = 1.0f, bool _isOutside = true) {
        origin = _origin;
        direction = _direction.normalized();
        intensity = _intensity;
        isOutside = _isOutside;
    }

    Ray(const Ray &ray) {
        origin = ray.origin;
        direction = ray.direction.normalized();
        intensity = ray.intensity;
        isOutside = ray.isOutside;
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

    const float& getIntensity() const {
        return intensity;
    }

    void setIntensity(const float& _intensity){
        intensity = _intensity;
        return;
    }

    Vector3f pointAtParameter(float t) const {
        return origin + direction * t;
    }
};

inline std::ostream &operator<<(std::ostream &os, const Ray &ray) {
    os << "Ray <" << ray.getOrigin() << ", " << ray.getDirection()
        << ", " << ray.getIntensity() << ", " << ray.isOutside << ">";
    return os;
}

#endif // RAY_H
