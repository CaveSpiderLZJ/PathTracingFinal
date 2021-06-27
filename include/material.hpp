#ifndef MATERIAL_H
#define MATERIAL_H

#include <cassert>
#include <vecmath.h>

#include "ray.hpp"
#include "hit.hpp"
#include "image.hpp"

#include "stb_image.h"

#include <iostream>
#include <cmath>
#include <cstring>

class Fresnel{
public:
    float fbase;
    float fscale;
    float power;

    Fresnel();

    Fresnel(const float& _fbase, const float& _fscale, const float& _power);
};

// TODO: Implement Shade function that computes Phong introduced in class.
class Material {

public:

    Vector3f diffuseColor;
    Vector3f specularColor;
    Vector3f luminance;
    float shininess;
    float refractiveIndex;
    Fresnel fresnel;
    unsigned char* texture;
    float textureScale;
    float textureLuminance;
    int w, h, n;

    Material() = delete;

    explicit Material(const Vector3f &_diffuseColor, const Vector3f &_specularColor = Vector3f::ZERO,
        const Vector3f& _luminance = Vector3f::ZERO, float _shininess = 0, float _refractiveIndex = 1.0f,
        Fresnel _fresnel = Fresnel(), const char* filePath = "\0",
        float textureScale = 1.0f, float textureLuminance = 0.0f);

    virtual ~Material() = default;

    virtual Vector3f getDiffuseColor(float u = 0.0f, float v = 0.0f) const;

    virtual Vector3f getLuminance(float u = 0.0f, float v = 0.0f) const;

    virtual Vector3f getDeltaNormal(const Vector3f& normal, float u, float v);

    Vector3f Shade(const Ray &ray, const Hit &hit,
        const Vector3f &dirToLight, const Vector3f &lightColor);
};

#endif // MATERIAL_H
