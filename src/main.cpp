#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <sys/time.h>
#include <iostream>
#include <iomanip>

#include "scene_parser.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "group.hpp"
#include "light.hpp"

#include <string>
#include <queue>
#include "omp.h"

#define MAX_DEPTH 8            // 光线跟踪最大迭代次数
#define RR 4                    // 俄罗斯轮盘赌终结
#define TMIN 1e-3
#define DELTA 1e-5
#define PROGRESS_NUM 10         // 画图时进度信息数目 
#define SAMPLING_TIMES 10     // 蒙特卡洛光线追踪采样率
#define THREAD_NUM 10           // 线程数

int randType(const float& reflectIntensity, const float& refractIntensity){
    // 输入折射率，反射率，用轮盘赌决定光线种类，折射反射漫反射返回012
    float p = float(rand()) / RAND_MAX;
    int type;
    if(p <= reflectIntensity) type = 0;
    else if(p <= reflectIntensity + refractIntensity) type = 1;
    else type = 2;
    return type;
}

Vector3f randDirection(const Vector3f& normal){
    // 输入一个法向，返回一个在法向所指的半球上的随机单位向量
    float theta = 2 * M_PI * rand() / RAND_MAX;
    float r = float(rand()) / RAND_MAX;
    float rs = sqrt(r);
    Vector3f u = (Vector3f::cross((normal[0]) > 0.1 ? Vector3f(0, 1, 0) : Vector3f(1, 0, 0), normal)).normalized();
    Vector3f v = Vector3f::cross(normal, u); 
    Vector3f res = Vector3f::ZERO;
    return (rs*cos(theta)*u + rs*sin(theta)*v + normal*sqrt(1-r)).normalized();
}

Vector3f clampedColor(Vector3f color){
    for(int i = 0; i < 3; i++)
        color[i] = color[i] < 0 ? 0 : (color[i] > 1 ? 1 : color[i]);
    return color;
}

void mcRayTracing(std::string inputFile, Image* img, int threadID){
    // 蒙特卡罗光线追踪
    SceneParser sceneParser(inputFile.c_str());
    Camera* camera = sceneParser.getCamera();
    Group* baseGroup = sceneParser.getGroup();
    // 定义绘图边界
    int startX = (float(threadID) / THREAD_NUM) * camera->getWidth();
    int endX;
    if(threadID == THREAD_NUM - 1)
        endX = camera->getWidth();
    else endX = (float(threadID + 1) / THREAD_NUM) * camera->getWidth();
    int progress = (endX - startX) / PROGRESS_NUM;
    for(int x = startX; x < endX; x++){
        if((x + 1 - startX) % progress == 0){
            std::cout << std::fixed << std::setprecision(2) <<
                 "### thread " << threadID << ' ' << std::fixed << std::setprecision(3)
                    << float(x-startX) / (endX-startX) << " finished." << std::endl;
        }
        for(int y = 0; y < camera->getHeight(); y++){
            Vector3f color = Vector3f::ZERO;
            for(int _ = 0; _ < SAMPLING_TIMES; _++){
                // 蒙特卡洛采样SAMPLING_TIMES次
                Ray currentRay = camera->generateRay(Vector2f(x + float(rand()) / RAND_MAX - 0.5f,
                    y + float(rand()) / RAND_MAX - 0.5f));
                while(true){
                    // 进行一次采样，直接加到color中
                    Hit hit;
                    bool isIntersect = baseGroup->intersect(currentRay, hit, TMIN);
                    Vector3f normal = hit.normal.normalized();
                    if(isIntersect){
                        Vector3f origin = currentRay.pointAtParameter(hit.t);
                        Vector3f foot = origin + normal * 
                            Vector3f::dot(currentRay.origin - origin, normal);
                        Vector3f reflectDirection = currentRay.direction - 2 * normal *
                            Vector3f::dot(currentRay.direction, normal);
                        float reflectIntensity = 0.0f, refractIntensity = 0.0f;
                        Vector3f refractDirection;
                        Material* material = hit.material;
                        Fresnel fresnel = material->fresnel;
                        float dotIN = -1 * Vector3f::dot(currentRay.direction.normalized(), normal);
                        bool reflectIsOutside = true;
                        if(currentRay.isOutside){
                            //光疏到光密，正常计算
                            reflectIsOutside = true;
                            reflectIntensity = fresnel.fbase + fresnel.fscale * pow((1.0f - dotIN), fresnel.power);
                            refractIntensity = fresnel.fbase + fresnel.fscale - reflectIntensity;
                            float sinS = sqrt(1.0f - dotIN * dotIN) / material->refractiveIndex;
                            float tanS = sqrt((sinS*sinS) / (1.0f - sinS*sinS));
                            refractDirection = (tanS * ((foot - currentRay.origin).normalized())
                                - normal).normalized();
                        }
                        else{
                            // 光密到光疏，注意临界折射角
                            reflectIsOutside = false;
                            float sinS = sqrt(1.0f - dotIN * dotIN) * material->refractiveIndex;
                            if(sinS >= 1.0f){
                                // 到达临界角，发生全反射
                                refractIntensity = 0.0f;
                                reflectIntensity = fresnel.fbase + fresnel.fscale;
                            }
                            else{
                                // 没有全反射，定义线性放缩倍数eta
                                float eta = 1.0f / pow(1.0f - sqrt(1.0f - (1.0f / 
                                    ((material->refractiveIndex)*(material->refractiveIndex)))), fresnel.power);
                                reflectIntensity = fresnel.fbase + fresnel.fscale *
                                    eta * pow(1.0f - dotIN, fresnel.power);
                                refractIntensity = fresnel.fbase + fresnel.fscale - reflectIntensity;
                                float tanS = sqrt((sinS*sinS) / (1.0f - sinS*sinS));
                                refractDirection = (tanS * ((foot - currentRay.origin).normalized())
                                    - normal).normalized();
                            }
                        }
                        float diffuseIntensity = 1.0f - reflectIntensity - refractIntensity;
                        // 至此，折射率、反射率、折射方向和反射方向已经计算完毕
                        // 根据当前物体亮度更新tmpColor
                        float ratio = 1.0f;
                        if(++currentRay.depth > RR){
                            Vector3f c = material->diffuseColor;
                            float rgbMax = (c[0] > c[1]) ? (c[0] > c[2] ? c[0] : c[2])
                                : (c[1] > c[2] ? c[1] : c[2]);
                            if(float(rand()) / RAND_MAX < rgbMax && currentRay.depth <= MAX_DEPTH)
                                ratio = 1 / rgbMax;
                            else{
                                color += material->luminance * currentRay.pastColor;
                                break;
                            }
                        }
                        int type = randType(reflectIntensity, refractIntensity);
                        color += material->luminance * currentRay.pastColor;
                        currentRay.pastColor = currentRay.pastColor * material->diffuseColor * ratio;  
                        // 轮盘赌确定下一次光线是反射折射还是漫反射
                        if(type == 0){
                            // 0 反射光线
                            currentRay.origin = origin + DELTA * reflectDirection;
                            currentRay.direction = reflectDirection;
                            currentRay.isOutside = reflectIsOutside;
                        }
                        else if(type == 1){
                            // 1 折射光线
                            currentRay.origin = origin + DELTA * refractDirection;
                            currentRay.direction = refractDirection;
                            currentRay.isOutside = !reflectIsOutside;
                        }
                        else{
                            // 2 漫反射光线
                            // 决定漫反射光线方向
                            Vector3f diffuseDirection = randDirection(normal);
                            currentRay.origin = origin + DELTA * diffuseDirection;
                            currentRay.direction = diffuseDirection;
                            currentRay.isOutside = reflectIsOutside;
                        }
                    }
                    else{
                        // 没有交点，返回背景色
                        color += currentRay.pastColor * sceneParser.getBackgroundColor();
                        break;
                    }
                }
            }
            color =  color / SAMPLING_TIMES;
            img->SetPixel(x, y, color);
        }
    }
}

int main(int argc, char *argv[]) {
    for (int argNum = 1; argNum < argc; ++argNum) {
        std::cout << "Argument " << argNum << " is: " << argv[argNum] << std::endl;
    }
    if (argc != 3) {
        std::cout << "Usage: ./bin/PA1 <input scene file> <output bmp file>" << std::endl;
        return 1;
    }
    std::string inputFile = argv[1];
    std::string outputFile = argv[2];  // only bmp is allowed.

    srand((unsigned int)time(0));
    SceneParser sceneParser(inputFile.c_str());
    Camera* camera = sceneParser.getCamera();
    Image img(camera->getWidth(), camera->getHeight());
    // 循环屏幕空间的像素
    struct timeval start, end; 
    gettimeofday(&start, NULL);
    #pragma omp parallel num_threads(THREAD_NUM)
    {
        int threadID = omp_get_thread_num();
        mcRayTracing(inputFile, &img, threadID);
    }
    gettimeofday(&end, NULL);
    float timecost = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/1e6;
    std::cout << "### time cost: " << timecost << " s" << std::endl;
    img.SaveBMP(outputFile.c_str());
    return 0;
}
