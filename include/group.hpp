#ifndef GROUP_H
#define GROUP_H


#include "object3d.hpp"
#include "ray.hpp"
#include "hit.hpp"
#include <iostream>
#include <vector>


// TODO: Implement Group - add data structure to store a list of Object*
class Group : public Object3D {
    
public:

    std::vector<Object3D*> objects;
    int num;

    Group() {   
        objects = std::vector<Object3D*>(0);
        num = 0;
        return;
    }

    explicit Group (int num_objects) {
        objects = std::vector<Object3D*>(0);
        num = num_objects;
        return;
    }

    ~Group() override {

    }

    bool intersect(const Ray &ray, Hit &hit, float tmin, float& u, float& v) override {
        float t = 1e38;
        Vector3f n;
        Material* m = nullptr;
        bool isIntersected = false;
        for(Object3D* object : objects){
            float tmpU = 0.0f, tmpV = 0.0f;
            if(object->intersect(ray, hit, tmin, tmpU, tmpV) && hit.t < t){
                isIntersected = true;
                t = hit.t;
                n = hit.normal;
                m = hit.material;
                u = tmpU; v = tmpV;
            }
        }
        if(!isIntersected) return false;
        hit.set(t, m, n);
        return true;
    }

    void addObject(int index, Object3D *obj) {
        objects.insert(objects.begin() + index, obj);
        return;
    }

    int getGroupSize() {
        return num;
    }
};

#endif
	
