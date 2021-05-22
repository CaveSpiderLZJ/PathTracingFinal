#ifndef MESH_H
#define MESH_H

#include <vector>
#include "object3d.hpp"
#include "triangle.hpp"
#include "Vector2f.h"
#include "Vector3f.h"

using namespace std;

class BSPNode{

public:

    // 节点包含的所有三角形编号，对应Mesh::trangles
    vector<int> triangleIdx;
    // 空间二分树父亲和两个孩子
    BSPNode* father;
    BSPNode* leftChild;
    BSPNode* rightChild;
    // x, y, z的最小最大值
    Vector3f minPos;
    Vector3f maxPos;
    // 本节点的孩子节点根据哪一维度来划分，012 for xyz
    int nextPartition;

    BSPNode() = delete;

    BSPNode(const Vector3f& _minPos, const Vector3f& _maxPos,
        int _nextPartition, BSPNode* _father = nullptr){
        // 创建BSP节点的时候就定义好其包围盒的范围
        leftChild = nullptr;
        rightChild = nullptr;
        minPos = _minPos;
        maxPos = _maxPos;
        nextPartition = _nextPartition;
        father = _father;
        return;
    }

    bool intersect(const Ray& ray){
        // 判断光线是否与节点的包围盒相交
        Vector3f origin = ray.origin;
        Vector3f direction = ray.direction;
        float minTmax = 1e38;
        float maxTmin = -1e38;
        for(int i = 0; i < 3; i++){
            // 遍历每一维，和平行面求交，更新minTmax和maxTmin
            bool isPositive = (direction[i] > 0);
            float tmin, tmax;
            if(isPositive){
                tmin = (minPos[i] - origin[i]) / direction[i];
                if(tmin > maxTmin) maxTmin = tmin;
                tmax = (maxPos[i] - origin[i]) / direction[i];
                if(tmax < minTmax) minTmax = tmax;
            }
            else{
                tmin = (maxPos[i] - origin[i]) / direction[i];
                if(tmin > maxTmin) maxTmin = tmin;
                tmax = (minPos[i] - origin[i]) / direction[i];
                if(tmax < minTmax) minTmax = tmax;
            }
        }
        return (minTmax >= maxTmin);
    }

    void getBounding(Vector3f& outMinPos, Vector3f& outMaxPos){
        // 获取本节点的包围盒
        outMinPos = minPos;
        outMaxPos = maxPos;
        return;
    }
};

class Mesh : public Object3D {

private:

    // Normal can be used for light estimation
    void computeNormal();

public:

    struct TriangleIndex {
        int x[3]{};
        TriangleIndex() {
            x[0] = 0; x[1] = 0; x[2] = 0;
        }
        int &operator[](const int i) { return x[i]; }
        // By Computer Graphics convention, counterclockwise winding is front face
    };

    vector<Vector3f> vertices;
    vector<TriangleIndex> triangles;
    vector<Vector3f> normals;
    BSPNode* root;

    bool intersect(const Ray &r, Hit &h, float tmin) override;
    Mesh(const char *filename, Material *m);

    void initBSP();      // 初始化BST树
    void computeTriangle(BSPNode* node);
};

#endif
