#ifndef CAMERA_H
#define CAMERA_H

#include "ray.hpp"
#include <vecmath.h>
#include <float.h>
#include <cmath>


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
    virtual Ray generateRay(const Vector2f &point) = 0;
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

public:
    PerspectiveCamera(const Vector3f &center, const Vector3f &direction,
            const Vector3f &up, int imgW, int imgH, float _angle) : Camera(center, direction, up, imgW, imgH) {
        // angle is in radian.
        this->angle = _angle;
        dis = (this->width / 2.0f) / tan(this->angle / 2.0f);
    }

    Ray generateRay(const Vector2f &point) override {
        // 产生一个intensity为1的光线
        Vector3f intersect = this->center + direction * this->dis;  // center in screen
        intersect += (point[0] - this->width / 2.0f) * this->horizontal;
        intersect += (point[1] - this->height / 2.0f) * this->up;
        return Ray(this->center, (intersect - this->center).normalized(), 1.0f, true);
    }
};

#endif //CAMERA_H
