#ifndef PINKYPI_SCENE_H
#define PINKYPI_SCENE_H

#include <string>
#include <vector>
#include <memory>
#include "pptypes.h"
#include "ray.h"
#include "intersection.h"

namespace PinkyPi {
    
    class AssetLibrary;
    class Node;
    class Camera;
    class Mesh;
    class Skin;
    class Light;
    class TracableStructure;
    class Config;
    class Texture;
    class BVH;
    
    /////
    class Scene {
    public:
        static Scene* buildDefaultScene();
        
//    private:
        AssetLibrary* assetLib;
        std::vector<Node*> topLevelNodes;
        std::vector<Node*> containsNodes;
        
        std::unique_ptr<BVH> objectBVH;

    public:
        // for trace
        std::vector<Node*> tracables;
        std::vector<Node*> lights;
        std::vector<Node*> cameras;
        
        Texture* backgroundTexture;
        
    public:
        Scene(AssetLibrary* al);
        
        bool preprocess(Config* config);
        
        // ### About open/clise time, timerate and slice. ###
        // opentime [t0]                                [t1]closetime
        // timerate [0.0]                               [1.0]
        // slice(4) [0]         [1]         [2]         [3]
        //           |-----------|-----------|-----------|
        //
        void seekTime(PPTimeType opentime, PPTimeType closetime, int slice, int storeId);
        PPFloat intersection(const Ray& ray, PPFloat hitnear, PPFloat hitfar, PPTimeType timerate, SceneIntersection *oisect) const;
        void computeIntersectionDetail(const Ray& ray, PPFloat hitt, PPTimeType timerate, const SceneIntersection& isect, IntersectionDetail* odetail) const;
        
    private:
        void preprocessTraverse(Node *node, Matrix4 gm, Config* config);
        void buildAccelerationStructure(int storeId);
    };
}

#endif
