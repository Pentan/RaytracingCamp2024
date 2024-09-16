#ifndef PINKYPI_NODE_H
#define PINKYPI_NODE_H

#include <string>
#include <vector>
#include <memory>
#include "pptypes.h"

namespace PinkyPi {
    
    class Camera;
    class Mesh;
    class Skin;
    class Light;
    class TracableStructure;
    
    class Node {
    public:
        enum ContentType {
            kContentTypeEmpty,
            kContentTypeCamera,
            kContentTypeMesh,
            kContentTypeLight
        };

        enum AnimatedFlags {
            kAnimatedNone   = 0,
            kAnimatedDirect = 1,
            kAnimatedInTree = 2
        };
        
        struct Transform {
            Matrix4 globalMatrix;
            Matrix4 matrix;
            Quaterion rotation;
            Vector3 scale;
            Vector3 translate;

            void makeMatrix();
            static Transform interpolate(const Transform& tf0, const Transform& tf1, PPFloat t);
        };

    public:
        Node(int i);
        ~Node();
        
        Matrix4 computeGlobalMatrix(PPTimeType tr) const;
        
        std::string name;
        int index;
        std::vector<int> children;
        
        ContentType contentType;
        union {
            Camera* camera;
            struct {
                Mesh* mesh;
                Skin* skin;
            };
            Light* light;
        } content;
        std::unique_ptr<TracableStructure> tracable;
        int animatedFlag;
        
        Node* parent;
        bool isTransformDirty;
        Transform initialTransform;
        Transform currentTransform;
        Matrix4 currentInverseGlobal;
        std::vector<Transform> transformCache;
    };
}

#endif
