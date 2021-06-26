#ifndef SPHERE_H
#define SPHERE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

// TODO: Implement functions and add more fields as necessary

class Sphere : public Object3D {

protected:
    Vector3f center;
    float radius;

public:
    Sphere() {
        // unit ball at the center
        center = Vector3f(0, 0, 0);
        radius = 1;
        return;
    }

    Sphere(const Vector3f &_center, float _radius, Material *_material) : Object3D(_material) {
        //
        center = _center;
        radius = _radius;
        return;
    }

    ~Sphere() override = default;

    bool intersect(const Ray &ray, Hit &hit, float tmin, float& u, float& v) override {
        // check if camera is in the sphere
        bool isAhead;
        float disCenter2Ray = disPoint2Ray(center, ray, isAhead);
        if(disCenter2Ray > radius) return false;
        float disCenter2Origin = disPoint2Point(center, ray.origin);
        float t;
        Vector3f intersect;
        Vector3f normal;
        float disFoot2Intersect = sqrt(radius * radius - disCenter2Ray * disCenter2Ray);
        float disFoot2Origin = sqrt(disCenter2Origin * disCenter2Origin - disCenter2Ray * disCenter2Ray);
        if(disCenter2Origin > radius){
            // origin outside the sphere
            if(!isAhead) return false;
            else{
                t = (disFoot2Origin - disFoot2Intersect);
                if(t < tmin) return false;
                intersect = ray.origin + t * ray.direction;
                normal = (intersect - center).normalized();
            }
        }
        else{
            // origin inside the sphere 
            if(isAhead) t = (disFoot2Origin + disFoot2Intersect);
            else t = (disFoot2Intersect - disFoot2Origin);
            if(t < tmin)return false;
            intersect = ray.origin + t * ray.direction;
            normal = (center - intersect).normalized();
        }
        Vector3f dir = intersect - center;
        if(dir[0] > 0.0f) u = 0.25f + atan(intersect[1] / intersect[0]) / (2 * M_PI);
        else u = 0.75f + atan(intersect[1] / intersect[0]) / (2 * M_PI);
        v = 0.5f - atan(intersect[2] / sqrt(intersect[0] * intersect[0] + intersect[1] * intersect[1])) / M_PI;
        hit.set(t, material, normal);
        return true;
    }
};


#endif
