#ifndef LIGHT_H
#define LIGHT_H

#include <Vector3f.h>
#include "object3d.hpp"

class Light {
public:
    Light() = default;

    virtual ~Light() = default;

    virtual void getIllumination(const Vector3f &p, Vector3f &dir,
        Vector3f &col, float& dis) const = 0;
};


class DirectionalLight : public Light {
public:
    DirectionalLight() = delete;

    DirectionalLight(const Vector3f &d, const Vector3f &c) {
        direction = d.normalized();
        color = c;
    }

    ~DirectionalLight() override = default;

    ///@param p unsed in this function
    ///@param distanceToLight not well defined because it's not a point light
    void getIllumination(const Vector3f &p, Vector3f &dir,
        Vector3f &col, float& dis) const override {
        // the direction to the light is the opposite of the
        // direction of the directional light source
        // dis 返回p到光源的距离，平行光默认来自正无穷远
        dir = -direction;
        col = color;
        dis = 1e38;
    }

private:

    Vector3f direction;
    Vector3f color;

};

class PointLight : public Light {
public:
    PointLight() = delete;

    PointLight(const Vector3f &p, const Vector3f &c) {
        position = p;
        color = c;
    }

    ~PointLight() override = default;

    void getIllumination(const Vector3f &p, Vector3f &dir,
        Vector3f &col, float& dis) const override {
        // the direction to the light is the opposite of the
        // direction of the directional light source
        // dis返回p到点光源的距离
        dir = (position - p);
        dis = dir.length();
        dir.normalize();
        col = color;
    }

private:

    Vector3f position;
    Vector3f color;

};

#endif // LIGHT_H
