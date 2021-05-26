#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <ctime>
#include <iostream>
#include <iomanip>

#include "scene_parser.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "group.hpp"
#include "light.hpp"

#include <string>
#include <queue>

#define MAX_ITER 16             // 光线跟踪最大迭代次数
#define MIN_INTENSITY 0.01      // 光线跟踪最小光强
#define TMIN 0.0001
#define DELTA 0.001
#define PROGRESS_NUM 20         // 画图时进度信息数目 
#define SAMPLING_TIMES 10       // 蒙特卡洛光线追踪采样率

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
    Vector3f res = Vector3f::ZERO;
    do{
        for(int i = 0; i < 3; i++)
            res[i] = float(rand()) / (RAND_MAX >> 1) - 1.0f;
    }while(Vector3f::dot(res, normal) <= 0);
    return res.normalized();
}

void phongRayTracing(SceneParser* sceneParser, Camera* camera, Image* img){
    // 使用Phong模型加简单光线追踪渲染img
    int progress = camera->getWidth() / PROGRESS_NUM;
    for (int x = 0; x < camera->getWidth(); ++x){
        if((x + 1) % progress == 0){
            std::cout << std::fixed << std::setprecision(2) <<
                 "### progress: " << float(x) / camera->getWidth() << std::endl;
        }
        for (int y = 0; y < camera->getHeight(); ++y){
            Vector3f color = Vector3f::ZERO;
            Ray camRay = sceneParser->getCamera()->generateRay(Vector2f(x, y));
            Group* baseGroup = sceneParser->getGroup();
            // 开一个队列，储存待处理的光线，最开始只有相机视线
            std::queue<Ray> rays;
            rays.push(camRay);
            // 循环对队列中每束光线求交，每次求交并shade产生一个颜色和反射折射两束光线
            // 光束能量分成反射、折射和吸收三部分，每次用吸收权值乘以渲染出来的颜色累加到color中
            // 用反射和折射权值生成新的光线加入队列
            while(rays.empty() == false){
                Ray currentRay = rays.front();
                Hit hit;
                bool isIntersect = baseGroup->intersect(currentRay, hit, TMIN);
                Vector3f normal = hit.normal.normalized();
                if(isIntersect){
                    // 当前光线和场景有交点
                    Vector3f tmpColor = Vector3f::ZERO;
                    // 找到交点之后，累加来自所有光源的光强影响
                    for (int lightIdx = 0; lightIdx < sceneParser->getNumLights(); ++lightIdx) {
                        Light* light = sceneParser->getLight(lightIdx);
                        Vector3f dir2Light, lightColor;
                        float distance;
                        // 获得光照强度
                        light->getIllumination(currentRay.pointAtParameter(hit.t),
                            dir2Light, lightColor, distance);
                        Hit tmpHit;
                        Ray tmpRay(currentRay.pointAtParameter(hit.t), dir2Light);
                        if(!(baseGroup->intersect(tmpRay, tmpHit, TMIN) && tmpHit.t < distance)){
                            // 计算局部光强
                            tmpColor += hit.material->Shade(currentRay, hit, dir2Light, lightColor);
                        }
                    }
                    // 计算反射光线和折射光线，加入队列
                    Vector3f origin = currentRay.pointAtParameter(hit.t);
                    // 计算反射光线方向，两种情况一样
                    Vector3f foot = origin + normal * 
                        Vector3f::dot(currentRay.origin - origin, normal);
                    Vector3f reflectDirection = (2 * foot - currentRay.origin - origin).normalized();
                    // 计算折射光线方向、反射光强和折射光强，分两种情况讨论
                    // 反射率 + 折射率 + 透射率 == 1
                    float reflectIntensity = 0.0f;
                    float refractIntensity = 0.0f;
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
                    color += (1.0f - reflectIntensity - refractIntensity) * 
                        currentRay.intensity * tmpColor;
                    // 根据光强筛选出可用的光线加入队列
                    if(currentRay.intensity * reflectIntensity >= MIN_INTENSITY){
                        rays.push(Ray(origin + DELTA * reflectDirection, reflectDirection,
                            currentRay.intensity * reflectIntensity, reflectIsOutside));
                    }
                    else if(reflectIsOutside){
                        color += reflectIntensity * currentRay.intensity * tmpColor;
                    }
                    if(currentRay.intensity * refractIntensity >= MIN_INTENSITY){
                        rays.push(Ray(origin + DELTA * refractDirection, refractDirection,
                            currentRay.intensity * refractIntensity, !reflectIsOutside));
                    }
                    else if(!reflectIsOutside){
                        color += refractIntensity * currentRay.intensity * tmpColor;
                    }
                }   
                else{
                    // 当前光线和场景没有交点
                    color += currentRay.getIntensity() * sceneParser->getBackgroundColor();
                }
                rays.pop();
            }
            img->SetPixel(x, y, color);
        }
    }
}

void mcRayTracing(SceneParser* sceneParser, Camera* camera, Image* img){
    // 蒙特卡罗光线追踪
    int progress = camera->getWidth() / PROGRESS_NUM;
    Group* baseGroup = sceneParser->getGroup();
    for(int x = 0; x < camera->getWidth(); x++){
        if((x + 1) % progress == 0){
            std::cout << std::fixed << std::setprecision(2) <<
                 "### progress: " << float(x) / camera->getWidth() << std::endl;
        }
        for(int y = 0; y < camera->getHeight(); y++){
            Vector3f color = Vector3f::ZERO;
            std::vector<Vector3f> colors;   // 存放每次采样得到的颜色
            for(int _ = 0; _ < SAMPLING_TIMES; _++){
                // 蒙特卡洛采样SAMPLING_TIMES次
                Vector3f tmpColor = Vector3f::ZERO;
                Ray currentRay = sceneParser->getCamera()->generateRay(Vector2f(x, y));
                while(true){
                    // 进行一次采样，结果储存在tmpColor中
                    Hit hit;
                    bool isIntersect = baseGroup->intersect(currentRay, hit, TMIN);
                    Vector3f normal = hit.normal.normalized();
                    if(isIntersect){
                        Vector3f origin = currentRay.pointAtParameter(hit.t);
                        Vector3f foot = origin + normal * 
                            Vector3f::dot(currentRay.origin - origin, normal);
                        Vector3f reflectDirection = (2 * foot - currentRay.origin - origin).normalized();
                        float reflectIntensity = 0.0f;
                        float refractIntensity = 0.0f;
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
                        int type = randType(reflectIntensity, refractIntensity);
                        if(type == 2){
                            currentRay.pastColor += currentRay.intensity * diffuseIntensity
                            * material->diffuseColor;
                        }
                        tmpColor += currentRay.intensity * diffuseIntensity
                            * material->luminance * currentRay.pastColor;
                        // 轮盘赌确定下一次光线是反射折射还是漫反射
                        if(type == 0){
                            // 0 反射光线
                            if(currentRay.intensity * reflectIntensity >= MIN_INTENSITY){
                                currentRay.origin = origin + DELTA * reflectDirection;
                                currentRay.direction = reflectDirection;
                                currentRay.intensity *= reflectIntensity;
                                currentRay.isOutside = reflectIsOutside;
                            }
                            else{
                                // intensity过小直接舍弃，下同
                                break;
                            }
                        }
                        else if(type == 1){
                            // 1 折射光线
                            if(currentRay.intensity * refractIntensity >= MIN_INTENSITY){
                                currentRay.origin = origin + DELTA * refractDirection;
                                currentRay.direction = refractDirection;
                                currentRay.intensity *= refractIntensity;
                                currentRay.isOutside = !reflectIsOutside;
                            }
                            else break;
                        }
                        else{
                            // 2 漫反射光线
                            if(currentRay.intensity * diffuseIntensity >= MIN_INTENSITY){
                                // 决定漫反射光线方向
                                Vector3f diffuseDirection = randDirection(normal);
                                // currentRay.pastColor += currentRay.intensity * diffuseIntensity
                                //      * material->diffuseColor;
                                currentRay.origin = origin + DELTA * diffuseDirection;
                                currentRay.direction = diffuseDirection;
                                currentRay.intensity *= diffuseIntensity;
                                currentRay.isOutside = reflectIsOutside;
                            }
                            else break;
                        }
                    }
                    else{
                        // 没有交点，返回背景色
                        tmpColor += currentRay.intensity * sceneParser->getBackgroundColor();
                        break;
                    }
                }
                colors.push_back(tmpColor);
            }
            // 计算colors的平均值
            // std::cout << "### size: " << colors.size() << std::endl;
            for(int i = 0; i < colors.size(); i++)
                color += colors[i];
            color =  color / colors.size();
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
    clock_t startTime = clock();
    mcRayTracing(&sceneParser, camera, &img);
    clock_t endTime = clock();
    std::cout<<"### time cost: "<< double(endTime - startTime) / CLOCKS_PER_SEC << " s" << std::endl;
    img.SaveBMP(outputFile.c_str());
    return 0;
}
