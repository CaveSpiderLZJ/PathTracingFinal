#ifndef CAMERA_H
#define CAMERA_H

#include "ray.hpp"
#include <vecmath.h>
#include <float.h>
#include <cmath>
#include <cstdlib>

class Camera {

protected:
    // Extrinsic parameters
    Vector3f center;
    Vector3f direction;
    Vector3f up;
    Vector3f horizontal;
    // Intrinsic parameters
    int width;
    int height;

public:
    Camera(const Vector3f &center, const Vector3f &direction, const Vector3f &up, int imgW, int imgH) {
        this->center = center;
        this->direction = direction.normalized();
        this->horizontal = Vector3f::cross(this->direction, up);
		this->horizontal.normalize();
        this->up = Vector3f::cross(this->horizontal, this->direction);
        this->width = imgW;
        this->height = imgH;
    }

    // Generate rays for each screen-space coordinate
    virtual Ray generateRay(const Vector2f &point, unsigned short* seed) = 0;
    virtual ~Camera() = default;

    int getWidth() const { return width; }
    int getHeight() const { return height; }


};

// TODO: Implement Perspective camera
// You can add new functions or variables whenever needed.
class PerspectiveCamera : public Camera {

private:
 
    float angle;
    float dis;
    float focus;    // 焦平面距离
    float field;    // 光圈大小

public:
    PerspectiveCamera(const Vector3f &center, const Vector3f &direction,
        const Vector3f &up, int imgW, int imgH, float _angle, float _focus = 0.0f , float _field = 0.0f)
        :Camera(center, direction, up, imgW, imgH){
        // angle is in radian.
        angle = _angle;
        focus = _focus;
        if(_focus == 0.0f) focus = dis;
        field = _field;
        dis = (width / 2.0f) / tan(angle / 2.0f);
    }

    Vector3f randDelta(unsigned short* seed){
        // return a random delta based on field
        float theta = 2 * M_PI * erand48(seed);
        float r1 = erand48(seed), r2 = erand48(seed);
        float r1s = sqrt(r1), r2s = sqrt(r2);
        float dir = 1.0f;
        if(erand48(seed) < 0.5f) dir = -1.0f;
        return (r1s*cos(theta)*Vector3f::UP + r1s*sin(theta)*Vector3f::RIGHT
            + dir*sqrt(1-r1)*Vector3f::FORWARD) * field * r2s;
    }

    Ray generateRay(const Vector2f &point, unsigned short* seed) override {
        Vector3f intersect = center + direction * focus;  // center in screen
        intersect += (point[0] - width / 2.0f) * (focus / dis) * horizontal;
        intersect += (point[1] - height / 2.0f) * (focus / dis) * up;
        Vector3f start = center;
        if(field != 0.0f) start += randDelta(seed);
        return Ray(start, (intersect - start).normalized(), Vector3f(1, 1, 1), 0);
    }
};

#endif //CAMERA_H
