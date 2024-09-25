#ifndef SPECTRENOTES_TRACABLESTRUCTURE_H
#define SPECTRENOTES_TRACABLESTRUCTURE_H

#include <memory>
#include <vector>
#include "types.h"
#include "ray.h"
#include "aabb.h"
#include "intersection.h"

namespace Spectrenotes {
    class Mesh;
    class MeshCache;
    class Skin;
    class Node;
    
    //
    class TracableStructure {
    public:
        Mesh* mesh;
        Node* ownerNode;
        Matrix4 invGlobalMatrix;
        AABB globalBounds; // dataId: index in scene

        TracableStructure(Node* owner, Mesh* m) : ownerNode(owner), mesh(m) {};
        virtual ~TracableStructure() {}
        
        virtual void initialize(int maxslice) = 0;
        virtual void clearSlice() = 0;
        virtual void updateSlice(int sliceId) = 0;
        virtual void updateFinished() = 0;
        virtual RTFloat intersection(const Ray& ray, RTFloat nearhit, RTFloat farhit, RTTimeType timerate, MeshIntersection* oisect) const = 0;
        virtual void intersectionDetail(const Ray& ray, RTFloat hitt, RTTimeType timerate, const MeshIntersection& isect, IntersectionDetail* odetail) const = 0;
    };
    
    //
    class StaticMeshStructure : public TracableStructure {
    public:
        std::unique_ptr<MeshCache> cache;

        Matrix4 globalMatrix;
        
        StaticMeshStructure(Node* owner, Mesh* m) : TracableStructure(owner, m) {};
        ~StaticMeshStructure() {};
        
        void initialize(int maxslice) override;
        void clearSlice() override;
        void updateSlice(int sliceId) override;
        void updateFinished() override;
        RTFloat intersection(const Ray& ray, RTFloat nearhit, RTFloat farhit, RTTimeType timerate, MeshIntersection* oisect) const override;
        void intersectionDetail(const Ray& ray, RTFloat hitt, RTTimeType timerate, const MeshIntersection& isect, IntersectionDetail* odetail) const override;
    };
    
    //
    class SkinMeshStructure : public TracableStructure {
    public:
        Skin* skin;
        std::unique_ptr<MeshCache> cache;
        std::vector<Matrix4> jointMatrices;
        std::vector<Matrix4> jointInvTransMatrices;
        
        SkinMeshStructure(Node* owner, Mesh* m, Skin* s) : TracableStructure(owner, m), skin(s) {};
        ~SkinMeshStructure() {};
        
        void initialize(int maxslice) override;
        void clearSlice() override;
        void updateSlice(int sliceId) override;
        void updateFinished() override;
        RTFloat intersection(const Ray& ray, RTFloat nearhit, RTFloat farhit, RTTimeType timerate, MeshIntersection* oisect) const override;
        void intersectionDetail(const Ray& ray, RTFloat hitt, RTTimeType timerate, const MeshIntersection& isect, IntersectionDetail* odetail) const override;
    };
}

#endif /* SPECTRENOTES_TRACABLESTRUCTURE_H */
