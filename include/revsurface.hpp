#ifndef REVSURFACE_HPP
#define REVSURFACE_HPP

#include "object3d.hpp"
#include "curve.hpp"
#include "mesh.hpp"
#include <tuple>

#define RESOLUTION 5
#define THETA_RES 20
#define ITER 10

class RevSurface : public Object3D {

    Curve* pCurve;
    Mesh* mesh;
    int m, n;   // 离散面片m行n列

public:
    RevSurface(Curve *pCurve, Material* material) : pCurve(pCurve), Object3D(material) {
        // Check flat.
        for (const auto &cp : pCurve->getControls()) {
            if (cp.z() != 0.0) {
                printf("Profile of revSurface must be flat on xy plane.\n");
                exit(0);
            }
        }
        std::vector<CurvePoint> data;
        pCurve->discretize(RESOLUTION, data);
        std::vector<Vector3f> v;
        for(int i = 0; i < THETA_RES; i++){
            Quat4f rot;
            rot.setAxisAngle(-float(i) / THETA_RES * 2 * M_PI, Vector3f::UP);
            for(int j = 0; j < data.size(); j++)
                v.push_back(Matrix3f::rotation(rot) * data[j].V);
        }
        m = THETA_RES;
        n = data.size();
        mesh = new Mesh(v, material, m, n);
    }

    ~RevSurface() override {
        delete pCurve;
    }

    bool intersect(const Ray &ray, Hit &hit, float tmin, float& u, float& v) override {
        // if(mesh->intersect(ray, hit, tmin, u, v)){
        //     // discreate mesh bounding has intersected with revsurface
        //     Vector3f inter = ray.pointAtParameter(hit.t);
        //     float theta = atan(inter[2] / inter[0]) + (inter[0] > 0.0f ? 
        //         (inter[2] > 0.0f ? 0.0f : 2.0f) : 1.0f) * M_PI;
        //     //std::cout << "### theta: " << theta << std::endl;
        //     float tu = float((mesh->interIdx[0] % n) + (mesh->interIdx[1] % n) + (mesh->interIdx[2] % n)) / (3 * n);
        //     //std::cout << "### tu: " << tu << std::endl;
        //     Vector3f x(hit.t, tu, theta);
        //     Vector3f dir = ray.direction;
        //     CurvePoint point = pCurve->pointAtPara(x[1]);
        //     for(int i = 0; i < ITER; i++){
        //         //std::cout << "### x: " << x[0] << ' ' << x[1] << ' ' << x[2] << std::endl;
        //         // Newton iteration
        //         //std::cout << "### tu: " << i << ' ' << x[1] << std::endl;
        //         float sinTheta = sin(x[2]), cosTheta = cos(x[2]);
        //         point = pCurve->pointAtPara(x[1]);
        //         Vector3f Fx = ray.origin + x[0] * dir
        //             - Vector3f(cosTheta * point.V[0], point.V[1], sinTheta * point.V[0]);
        //         Matrix3f Fprime(dir[0], -cosTheta * point.T[0], sinTheta * point.V[0],
        //                         dir[1], -point.T[1], 0.0f,
        //                         dir[2], -sinTheta * point.T[0], -cosTheta * point.V[0]);
        //         x = x - 0.05f * (Fprime.inverse() * Fx);
        //     }
        //     Vector3f normal = Vector3f::cross(point.T, -Vector3f::FORWARD).normalized();
        //     Quat4f rot;
        //     rot.setAxisAngle(-float(x[2]), Vector3f::UP);
        //     normal = Matrix3f::rotation(rot) * normal;
        //     if(Vector3f::dot(normal, dir) > 0.0f) normal = -normal;
        //     if(x[0] > tmin){
        //         hit.set(x[0], material, normal);
        //         return true;
        //     }
        //     return false;
        // }
        // else return false;
        return mesh->intersect(ray, hit, tmin, u, v);
    }
};

#endif //REVSURFACE_HPP
