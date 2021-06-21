#ifndef CURVE_HPP
#define CURVE_HPP

#include "object3d.hpp"
#include <vecmath.h>
#include <vector>
#include <utility>
#include <iostream>
#include <algorithm>
#include <cmath>
using namespace std;

// DONE (PA3): Implement Bernstein class to compute spline basis function.
//       You may refer to the python-script for implementation.

// The CurvePoint object stores information about a point on a curve
// after it has been tesselated: the vertex (V) and the tangent (T)
// It is the responsiblility of functions that create these objects to fill in all the data.
struct CurvePoint {
    Vector3f V; // Vertex
    Vector3f T; // Tangent  (unit)
};

class Bernstein{
public:
    int n;      // order
    double t;    // [0, 1]
    double* res;
    double* tpow;
    double* qpow;

    Bernstein(int _n, double _t): n(_n), t(_t){
        // 初始化
        res = new double[(n + 1) * (n + 1)];
        for(int i = 0; i < (n + 1) * (n + 1); i++){
            res[i] = -1.0;
        }
        // 计算端点值
        tpow = new double[n + 1];     // t^{n}
        qpow = new double[n + 1];     // (1-t)^{n}
        for(int i = 0; i <= n; i++){
            tpow[i] = pow(t, i);
            qpow[i] = pow(1 - t, i);
        }
        for(int j = 0; j <= n; j++){
            res[j] = qpow[j];
        }
        for(int i = 1; i <= n; i++){
            res[i * (n + 1) + i] = tpow[i];
        }
        // dp计算其他B_{i, n}
        for(int i = 1; i < n; i++){
            for(int j = i + 1; j <= n; j++){
                res[i * (n + 1) + j] = (1 - t) * res[i * (n + 1) + j - 1]
                    + t * res[(i - 1) * (n + 1) + j - 1];
            }
        }
        return;
    }

    ~Bernstein(){
        delete[] res;
        delete[] tpow;
        delete[] qpow;
    }

    double basis(int i, int p){
        return res[i * (n + 1) + p];
    }

    double dbasis(int i, int p){
        if(i <= 0)
            return -1 * p * qpow[p - 1];
        if(i == p){
            return p * tpow[p - 1];
        }
        return double(p) * (res[(i-1) * (n+1) + (p-1)] - res[i * (n+1) + (p-1)]);
    }
};

class BsplineBase{
public:
    int n, k;
    double t;
    double* res;

    BsplineBase(int _n, int _k, double _t): n(_n), k(_k), t(_t){
        // 初始化
        res = new double[(n + k + 1) * (k + 1)];
        for(int i = 0; i < (n + k + 1) * (k + 1); i++){
            res[i] = -1.0;
        }
        // 计算初值
        for(int i = 0; i <= n + k; i++){
            res[i * (k + 1)] = 0;
        }
        int index = int(t * (n + k + 1));
        res[index * (k + 1)] = 1;
        // dp计算B_{i, p}
        for(int j = 1; j <= k; j++){
            for(int i = 0; i <= n + k; i++){
                if(i + j > n + k)
                    continue;
                res[i * (k+1) + j] = ((t * (n+k+1) - i) / j) * res[i * (k+1) + j-1]
                 + ((i+j+1 - t * (n+k+1)) / j) * res[(i+1) * (k+1) + j-1];
            }
        }
        return;

    }

    double basis(int i, int p){
        return res[i * (k+1) + p];
    }
    
    double dbasis(int i, int p){
        return (n + k + 1) * (res[i * (k+1) + p-1] - res[(i+1) * (k+1) + p-1]);
    }
};

class Curve : public Object3D {
protected:
    std::vector<Vector3f> controls;
public:
    explicit Curve(std::vector<Vector3f> points) : controls(std::move(points)) {}

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        return false;
    }

    std::vector<Vector3f> &getControls() {
        return controls;
    }

    virtual void discretize(int resolution, std::vector<CurvePoint>& data) = 0;

    void drawGL() override {
        Object3D::drawGL();
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glDisable(GL_LIGHTING);
        glColor3f(1, 1, 0);
        glBegin(GL_LINE_STRIP);
        for (auto & control : controls) { glVertex3fv(control); }
        glEnd();
        glPointSize(4);
        glBegin(GL_POINTS);
        for (auto & control : controls) { glVertex3fv(control); }
        glEnd();
        std::vector<CurvePoint> sampledPoints;
        discretize(30, sampledPoints);
        glColor3f(1, 1, 1);
        glBegin(GL_LINE_STRIP);
        for (auto & cp : sampledPoints) { glVertex3fv(cp.V); }
        glEnd();
        glPopAttrib();
    }
};

class BezierCurve : public Curve {
public:
    explicit BezierCurve(const std::vector<Vector3f> &points) : Curve(points) {
        if (points.size() < 4 || points.size() % 3 != 1) {
            printf("Number of control points of BezierCurve must be 3n+1!\n");
            exit(0);
        }
    }

    void discretize(int resolution, std::vector<CurvePoint>& data) override {
        data.clear();
        // DONE (PA3): fill in data vector
        int N = controls.size() - 1;
        if(N < 1) return;
        for(int i = 0; i < N; i++){
            for(int j = 0; j < resolution; j++){
                // N + 1个控制点，要画N段，每段resolution个点
                // 参数t，映射到[0, 1]
                double t = double(i * resolution + j) / (N * resolution);
                Bernstein* bernstein = new Bernstein(N, t);
                CurvePoint point;
                point.V = Vector3f::ZERO;
                point.T = Vector3f::ZERO;
                for(int k = 0; k <= N; k++){
                    point.V += bernstein->basis(k, N) * controls[k];
                    double db = bernstein->dbasis(k, N);
                    point.T += bernstein->dbasis(k, N) * controls[k];
                }
                point.T.normalize();
                data.push_back(point);
                delete bernstein;
            }
        }
        return;
    }

protected:

};

class BsplineCurve : public Curve {
public:
    BsplineCurve(const std::vector<Vector3f> &points) : Curve(points) {
        if (points.size() < 4) {
            printf("Number of control points of BspineCurve must be more than 4!\n");
            exit(0);
        }
    }

    void discretize(int resolution, std::vector<CurvePoint>& data) override {
        data.clear();
        // DONE (PA3): fill in data vector
        int N = controls.size() - 1;
        int K = 3;      // default
        if(N < 1) return;
        for(int i = 0; i < N + 1 - K; i++){
            for(int j = 0; j < resolution; j++){
                // N + 1个控制点，要画N + 1 - K段，每段resolution个点
                // 参数t，映射到[t_{k-1}, t_{n+1}]，offset为K
                double t = double((K+i) * resolution + j) / ((N + K + 1) * resolution);
                BsplineBase* bsplineBase = new BsplineBase(N, K, t);
                CurvePoint point;
                point.V = Vector3f::ZERO;
                point.T = Vector3f::ZERO;
                for(int l = 0; l <= N; l++){
                    point.V += bsplineBase->basis(l, K) * controls[l];
                    point.T += bsplineBase->dbasis(l, K) * controls[l];
                }
                point.T.normalize();
                data.push_back(point);
                delete bsplineBase;
            }
        }
        return;
    }
};

#endif // CURVE_HPP
