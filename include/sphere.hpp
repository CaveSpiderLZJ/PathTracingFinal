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

    bool intersect(const Ray &ray, Hit &hit, float tmin) override {
        // check if camera is in the sphere
        bool isAhead;
        float disCenter2Ray = disPoint2Ray(center, ray, isAhead);
        if(disCenter2Ray > radius) return false;
        float disCenter2Origin = disPoint2Point(center, ray.getOrigin());
        float t;
        Vector3f intersect;
        Vector3f normal;
        float disFoot2Intersect = sqrt(radius * radius - disCenter2Ray * disCenter2Ray);
        float disFoot2Origin = sqrt(disCenter2Origin * disCenter2Origin - disCenter2Ray * disCenter2Ray);
        if(disCenter2Origin > radius){
            // origin outside the sphere
            if(!isAhead) return false;
            else{
                t = (disFoot2Origin - disFoot2Intersect) / ray.getDirection().normalized().length();
                if(t < tmin) return false;
                intersect = ray.getOrigin() + t * ray.getDirection().normalized();
                normal = (intersect - center).normalized();
            }
        }
        else{
            // origin inside the sphere 
            if(isAhead) t = (disFoot2Origin + disFoot2Intersect) / ray.getDirection().normalized().length();
            else t = (disFoot2Intersect - disFoot2Origin) / ray.getDirection().normalized().length();
            if(t < tmin)return false;
            intersect = ray.getOrigin() + t * ray.getDirection().normalized();
            normal = (center - intersect).normalized(); 
        }
        hit.set(t, material, normal);
        return true;
    }
};


#endif
