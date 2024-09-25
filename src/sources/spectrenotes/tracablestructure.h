#ifndef PINKYPI_TRACABLESTRUCTURE_H
#define PINKYPI_TRACABLESTRUCTURE_H

#include <memory>
#include <vector>
#include "pptypes.h"
#include "ray.h"
#include "aabb.h"
#include "intersection.h"

namespace PinkyPi {
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
        virtual PPFloat intersection(const Ray& ray, PPFloat nearhit, PPFloat farhit, PPTimeType timerate, MeshIntersection* oisect) const = 0;
        virtual void intersectionDetail(const Ray& ray, PPFloat hitt, PPTimeType timerate, const MeshIntersection& isect, IntersectionDetail* odetail) const = 0;
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
        PPFloat intersection(const Ray& ray, PPFloat nearhit, PPFloat farhit, PPTimeType timerate, MeshIntersection* oisect) const override;
        void intersectionDetail(const Ray& ray, PPFloat hitt, PPTimeType timerate, const MeshIntersection& isect, IntersectionDetail* odetail) const override;
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
        PPFloat intersection(const Ray& ray, PPFloat nearhit, PPFloat farhit, PPTimeType timerate, MeshIntersection* oisect) const override;
        void intersectionDetail(const Ray& ray, PPFloat hitt, PPTimeType timerate, const MeshIntersection& isect, IntersectionDetail* odetail) const override;
    };
}

#endif /* PINKYPI_TRACABLESTRUCTURE_H */
