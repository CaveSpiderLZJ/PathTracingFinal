#ifndef MATERIAL_H
#define MATERIAL_H

#include <cassert>
#include <vecmath.h>

#include "ray.hpp"
#include "hit.hpp"
#include <iostream>
#include <cmath>

class Fresnel{
public:
    float fbase;
    float fscale;
    float power;

    Fresnel(): fbase(0.0f), fscale(0.0f), power(1.0f) {}

    Fresnel(const float& _fbase, const float& _fscale, const float& _power):
        fbase(_fbase), fscale(_fscale), power(_power) {}
};

// TODO: Implement Shade function that computes Phong introduced in class.
class Material {

public:

    Vector3f diffuseColor;
    Vector3f specularColor;
    float shininess;
    float refractiveIndex;
    Fresnel fresnel;

    Material() = delete;

    explicit Material(const Vector3f &_diffuseColor, const Vector3f &_specularColor = Vector3f::ZERO,
        float _shininess = 0, float _refractiveIndex = 1.0f, Fresnel _fresnel = Fresnel()):
        diffuseColor(_diffuseColor), specularColor(_specularColor), 
        shininess(_shininess), refractiveIndex(_refractiveIndex), fresnel(_fresnel) {}

    virtual ~Material() = default;

    virtual Vector3f getDiffuseColor() const {
        return diffuseColor;
    }

    Vector3f Shade(const Ray &ray, const Hit &hit,
        const Vector3f &dirToLight, const Vector3f &lightColor){
        Vector3f shaded = Vector3f::ZERO;
        Vector3f dir = -1 * dirToLight.normalized();
        Vector3f normal = hit.getNormal().normalized();
        Vector3f reflect = dir - 2 * Vector3f::dot(dir, normal) * normal;
        shaded += Vector3f::dot(dirToLight.normalized(), normal) * lightColor * diffuseColor;
        float dotRV = Vector3f::dot(reflect, -1 * ray.getDirection().normalized());
        if(dotRV > 0){
            shaded += lightColor * specularColor * pow(dotRV, shininess);
        }
        return shaded;
    }
};

#endif // MATERIAL_H
