#include "material.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <cmath>

#define SCALE 0.5f

Fresnel::Fresnel(): fbase(0.0f), fscale(0.0f), power(1.0f) {}

Fresnel::Fresnel(const float& _fbase, const float& _fscale, const float& _power):
        fbase(_fbase), fscale(_fscale), power(_power) {}

// TODO: Implement Shade function that computes Phong introduced in class.

Material::Material(const Vector3f &_diffuseColor, const Vector3f &_specularColor,
    const Vector3f& _luminance, float _shininess, float _refractiveIndex,
    Fresnel _fresnel, const char* filePath, float _textureScale, int _uneven):
    diffuseColor(_diffuseColor), specularColor(_specularColor), luminance(_luminance),
    shininess(_shininess), refractiveIndex(_refractiveIndex), fresnel(_fresnel),
    textureScale(_textureScale), uneven(_uneven){
    if(filePath[0] != 0)
        texture = stbi_load(filePath, &w, &h, &n, 0);
    else texture = nullptr;
}

Vector3f Material::getDiffuseColor(float u, float v) const {
    if(texture == nullptr) return diffuseColor;
    u *= textureScale; v *= textureScale;
    u -= floor(u); v -= floor(v);
    int idx = n * w * int(v * h) + int(u * w) * n;
    if(idx < 0) idx = 0;
    if(idx > n * w * h - 3) idx = n * w * h - 3;
    float r = texture[idx + 0] / 256.0f;
    float g = texture[idx + 1] / 256.0f;
    float b = texture[idx + 2] / 256.0f;
    return Vector3f(r, g, b);
}

Vector3f Material::getDeltaNormal(const Vector3f& normal, float u, float v){
    if(uneven == 0) return Vector3f::ZERO;
    u *= textureScale; v *= textureScale;
    u -= floor(u); v -= floor(v);
    int row = u * w, col = v * h;
    if(row < 3) row = 3; if(row > w-4) row = w-4;
    if(col < 3) col = 3; if(col > h-4) col = h-4;
    // row和col是图像横纵坐标，不考虑通道数
    float filter[5] = {-1, -2, 0, 2, 1};
    float drow = 0, dcol = 0;
    for(int i = 0; i < 5; i++){
        int rowIdx = n * w * (col + i - 2) + row * n;
        int colIdx = n * w * col + (row + i - 2) * n;
        for(int j = 0; j < 3; j++){
            drow += texture[rowIdx + j] * filter[i];
            dcol += texture[colIdx + j] * filter[i];
        }
        drow = drow / (256.0f * 9);
        dcol = dcol / (256.0f * 9);
    }
    Vector3f w2 = (Vector3f::cross(normal[0] > 0.1f ? Vector3f(0, 1, 0) : Vector3f(1, 0, 0), normal)).normalized();
    Vector3f w1 = Vector3f::cross(w2, normal);
    return drow * SCALE * w1 + dcol * SCALE * w2;
}

Vector3f Material::Shade(const Ray &ray, const Hit &hit,
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
