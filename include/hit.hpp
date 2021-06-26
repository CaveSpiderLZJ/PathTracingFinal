#ifndef HIT_H
#define HIT_H

#include <vecmath.h>
#include "ray.hpp"

class Material;

class Hit {

public:

    float t;
    Material *material;
    Vector3f normal;

    // constructors
    Hit() {
        material = nullptr;
        t = 1e38;
        normal = Vector3f(1, 0, 0);
    }

    Hit(float _t, Material *m, const Vector3f &n) {
        t = _t;
        material = m;
        normal = n.normalized();
    }

    Hit(const Hit &hit) {
        t = hit.t;
        material = hit.material;
        normal = hit.normal;
    }

    // destructor
    ~Hit() = default;

    float getT() const {
        return t;
    }

    Material *getMaterial() const {
        return material;
    }

    const Vector3f &getNormal() const {
        return normal;
    }

    void set(float _t, Material *m, const Vector3f &n) {
        t = _t;
        material = m;
        normal = n;
    }
    
};

inline std::ostream &operator<<(std::ostream &os, const Hit &h) {
    os << "Hit <" << h.t << ", " << h.normal << ">";
    return os;
}

#endif // HIT_H
