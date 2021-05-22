#include "mesh.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <utility>
#include <sstream>
#include <queue>
using namespace std;

#define MIN_TRIANGLE 12      // 当BSP一个节点包含的三角形少于这个数时，不再扩展它

bool Mesh::intersect(const Ray &ray, Hit &hit, float tmin) {
    // 记录所有三角形是否已经求过交，防止重复求交浪费算力
    for(int i = 0; i < triangles.size(); i++){
        hasIntersected[i] = false;
    }
    // 所有可能有交点的三角形id，重复三角形不往里面加
    vector<int> candidates;
    // 有可能有交点的BSP树节点队列，队列中不一定所有包围盒都和光线有交点
    queue<BSPNode*> nodes;
    nodes.push(root);
    while(!nodes.empty()){
        BSPNode* currentNode = nodes.front();
        bool currentIntersected = currentNode->intersect(ray);
        if(currentNode == nullptr){
            cout << "!!! nullptr found in queue" << endl;
            nodes.pop();
            continue;
        }
        else if(currentIntersected){
            // 光线和当前节点的包围盒有交点
            if(currentNode->leftChild == nullptr && currentNode->rightChild == nullptr){
                // 当前节点为叶子结点
                for(int i = 0; i < currentNode->triangleIdx.size(); i++){
                    // 遍历当前节点的所有三角形编号，如果没有加入过，就加入candidates
                    int idx = currentNode->triangleIdx[i];
                    if(hasIntersected[idx] == false){
                        candidates.push_back(idx);
                        hasIntersected[idx] = true;
                    }
                }
            }
            else{
                // 当前节点不是叶子节点，将其两个孩子加入队列
                nodes.push(currentNode->leftChild);
                nodes.push(currentNode->rightChild);
            }
        }
        // 当前节点包围盒和光线不相交，直接弹出
        nodes.pop();
    }
    // 至此，获得全部可能相交的三角形编号
    float t = 1e38;
    Vector3f n;
    Material* m;
    bool isIntersected = false;
    for (int i = 0; i < int(candidates.size()); i++) {
        TriangleIndex& triIndex = triangles[candidates[i]];
        Triangle triangle(vertices[triIndex[0]],
                          vertices[triIndex[1]], vertices[triIndex[2]], material);
        if(triangle.intersect(ray, hit, tmin) && hit.getT() < t){
            isIntersected = true;
            t = hit.getT();
            n = hit.getNormal();
            m = hit.getMaterial();
        }
    }
    if(!isIntersected) return false;
    hit.set(t, m, n);
    return true;
}

Mesh::Mesh(const char *filename, Material *material) : Object3D(material) {

    // Optional: Use tiny obj loader to replace this simple one.
    std::ifstream f;
    f.open(filename);
    if (!f.is_open()) {
        std::cout << "Cannot open " << filename << "\n";
        return;
    }
    std::string line;
    std::string vTok("v");
    std::string fTok("f");
    std::string texTok("vt");
    char bslash = '/', space = ' ';
    std::string tok;
    int texID;
    while (true) {
        std::getline(f, line);
        if (f.eof()) {
            break;
        }
        if (line.size() < 3) {
            continue;
        }
        if (line.at(0) == '#') {
            continue;
        }
        std::stringstream ss(line);
        ss >> tok;
        if (tok == vTok) {
            Vector3f vec;
            ss >> vec[0] >> vec[1] >> vec[2];
            vertices.push_back(vec);
        } else if (tok == fTok) {
            if (line.find(bslash) != std::string::npos) {
                std::replace(line.begin(), line.end(), bslash, space);
                std::stringstream facess(line);
                TriangleIndex trig;
                facess >> tok;
                for (int ii = 0; ii < 3; ii++) {
                    facess >> trig[ii] >> texID;
                    trig[ii]--;
                }
                triangles.push_back(trig);
            } else {
                TriangleIndex trig;
                for (int ii = 0; ii < 3; ii++) {
                    ss >> trig[ii];
                    trig[ii]--;
                }
                triangles.push_back(trig);
            }
        } else if (tok == texTok) {
            Vector2f texcoord;
            ss >> texcoord[0];
            ss >> texcoord[1];
        }
    }
    f.close();
    initBSP();
    hasIntersected = new bool[triangles.size()];
}

Mesh::~Mesh(){
    delete[] hasIntersected;
    vector<BSPNode*> nodes;
    nodes.push_back(root);
    int head = 0, tail = 1;
    while(head < tail){
        BSPNode* currentNode = nodes[head];
        if(currentNode->leftChild != nullptr){
            nodes.push_back(currentNode->leftChild);
            nodes.push_back(currentNode->rightChild);
            tail += 2;
        }
        head++;
    }
    for(int i = nodes.size() - 1; i >= 0; i--){
        delete nodes[i];
    }
    return;
}

void Mesh::computeNormal() {
    normals.resize(triangles.size());
    for (int triId = 0; triId < (int) triangles.size(); ++triId) {
        TriangleIndex& triIndex = triangles[triId];
        Vector3f a = vertices[triIndex[1]] - vertices[triIndex[0]];
        Vector3f b = vertices[triIndex[2]] - vertices[triIndex[0]];
        b = Vector3f::cross(a, b);
        normals[triId] = b / b.length();
    }
}

void Mesh::initBSP(){
    // 根据已经初始化的三角形面片，建立BSP树
    Vector3f minPos = Vector3f(1e8, 1e8, 1e8);
    Vector3f maxPos = Vector3f(-1e8, -1e8, -1e8);
    for(int i = 0; i < vertices.size(); i++){
        for(int j = 0; j < 3; j++){
            if(vertices[i][j] < minPos[j])
                minPos[j] = vertices[i][j];
            if(vertices[i][j] > maxPos[j])
                maxPos[j] = vertices[i][j];
        }
    }
    root = new BSPNode(minPos, maxPos, 0);
    queue<BSPNode*> nodes;      // 待计算并扩展的节点队列
    nodes.push(root);
    while(!nodes.empty()){
        BSPNode* currentNode = nodes.front();
        computeTriangle(currentNode);
        if(currentNode->triangleIdx.size() > MIN_TRIANGLE){
            // 当前节点的三角形超过了下限，继续扩展
            int nextPartition = currentNode->nextPartition;
            currentNode->getBounding(minPos, maxPos);
            float mean = (minPos[nextPartition] + maxPos[nextPartition]) / 2.0f;
            float tmpMax = maxPos[nextPartition];
            maxPos[nextPartition] = mean;
            currentNode->leftChild = new BSPNode(minPos, maxPos, (nextPartition + 1) % 3, currentNode);
            nodes.push(currentNode->leftChild);
            minPos[nextPartition] = mean;
            maxPos[nextPartition] = tmpMax;
            currentNode->rightChild = new BSPNode(minPos, maxPos, (nextPartition + 1) % 3, currentNode);
            nodes.push(currentNode->rightChild);
        }
        nodes.pop();
    }
    return;
}

void Mesh::computeTriangle(BSPNode* node){
    // 根据node中的包围盒范围，计算包含在其中的所有三角形编号
    // 并更新node->triangleIdx
    node->triangleIdx.clear();
    BSPNode* father = node->father;
    Vector3f minPos, maxPos;
    node->getBounding(minPos, maxPos);
    if(father == nullptr){
        // 树根，用Mesh里的全部三角形算
        for(int i = 0; i < triangles.size(); i++){
            // i 就是三角形编号，检查这个三角形是否有某点在包围盒里
            bool inside = false;
            for(int j = 0; j < 3; j++){
                Vector3f pos = vertices[triangles[i][j]];
                if(minPos[0] <= pos[0] && pos[0] <= maxPos[0] &&
                    minPos[1] <= pos[1] && pos[1] <= maxPos[1] &&
                    minPos[2] <= pos[2] && pos[2] <= maxPos[2]){
                    inside = true;
                    break;
                }
            }
            if(inside) node->triangleIdx.push_back(i);
        }
    }
    else{
        // 内部节点所有三角形一定包含于其父亲，用父亲的三角形算
        for(int i = 0; i < father->triangleIdx.size(); i++){
            // father->triangleIdx[i]是三角形编号，检查这个三角形是否有某点在包围盒里
            bool inside = false;
            for(int j = 0; j < 3; j++){
                Vector3f pos = vertices[triangles[father->triangleIdx[i]][j]];
                if(minPos[0] <= pos[0] && pos[0] <= maxPos[0] &&
                    minPos[1] <= pos[1] && pos[1] <= maxPos[1] &&
                    minPos[2] <= pos[2] && pos[2] <= maxPos[2]){
                    inside = true;
                    break;
                }
            }
            if(inside) node->triangleIdx.push_back(father->triangleIdx[i]);
        }
    }
    return;
}
