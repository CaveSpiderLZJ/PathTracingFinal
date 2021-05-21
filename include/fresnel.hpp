#ifndef FRESNEL_H
#define FRESNEL_H

#include <iostream>
#include <vecmath.h>
#include "ray.hpp"
#include "hit.hpp"

class Fresnel{
public:
    float fbase;
    float fscale;
    float power;

    Fresnel(): fbase(0.0f), fscale(0.0f), power(1.0f) {}

    Fresnel(const float& _fbase, const float& _fscale, const float& _power):
        fbase(_fbase), fscale(_fscale), power(_power) {}
};

#endif