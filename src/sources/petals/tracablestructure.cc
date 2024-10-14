//
//  tracablestructure.cpp
//  Petals
//
//  Created by SatoruNAKAJIMA on 2022/08/14.
//

#include "tracablestructure.h"
#include "mesh.h"
#include "skin.h"
#include "node.h"
#include "ray.h"
#include "bvh.h"
#include "material.h"

using namespace Petals;

// StaticMeshStructure
void StaticMeshStructure::initialize(int maxslice) {
    if (ownerNode->animatedFlag == 0) {
        const auto& gm = ownerNode->initialTransform.globalMatrix;
        invGlobalMatrix = Matrix4::inverted(gm, nullptr);
        
        // tight bounds
        globalBounds.clear();
        for(auto ic = mesh->clusters.begin(); ic != mesh->clusters.end(); ++ic) {
            auto* c = ic->get();
            for(auto iv = c->vertices.begin(); iv != c->vertices.end(); ++iv) {
                auto v = Matrix4::transformV3(gm, *iv);
                globalBounds.expand(v);
            }
        }
    }
}

void StaticMeshStructure::clearSlice() {
    if (ownerNode->animatedFlag != 0) {
        globalBounds.clear();
    }
}

void StaticMeshStructure::updateSlice(int sliceId) {
    // update AABB
    if (ownerNode->animatedFlag != 0) {
        const auto& gm = ownerNode->currentTransform.globalMatrix;
        globalBounds.expand(AABB::transformed(mesh->bounds, gm));
    }
}

void StaticMeshStructure::updateFinished() {
    // TODO
}

RTFloat StaticMeshStructure::intersection(const Ray& ray, RTFloat nearhit, RTFloat farhit, RTTimeType timerate, MeshIntersection* oisect) const {
    Matrix4 gm;
    Matrix4 igm;
    
    if(!globalBounds.isIntersect(ray, nearhit, farhit)) {
        return -1.0;
    }

    if (ownerNode->animatedFlag == 0) {
        igm = invGlobalMatrix;
        gm = ownerNode->initialTransform.globalMatrix;
    } else {
        gm = ownerNode->computeGlobalMatrix(timerate);
        igm = Matrix4::inverted(gm, nullptr);
    }
    
    Ray lray = ray.transformed(igm);
    Vector3 lnearp = Matrix4::transformV3(igm, ray.pointAt(nearhit));
    RTFloat lnearhit = (lnearp - lray.origin).length();
    Vector3 lfarp = Matrix4::transformV3(igm, ray.pointAt(farhit));
    RTFloat lfarhit = (lfarp - lray.origin).length();
    
    RTFloat lt = mesh->intersection(lray, lnearhit, lfarhit, oisect);
    if(lt < lnearhit) {
        return -1.0;
    }
    
    Vector3 lhp = lray.pointAt(lt);
    Vector3 ghp = Matrix4::transformV3(gm, lhp);

    return (ray.origin - ghp).length();
}

void StaticMeshStructure::intersectionDetail(const Ray& ray, RTFloat hitt, RTTimeType timerate, const MeshIntersection& isect, IntersectionDetail* odetail) const {
    Matrix4 gm;
    Matrix4 igm;
    Matrix4 itgm;

    if (ownerNode->animatedFlag == 0) {
        gm = ownerNode->initialTransform.globalMatrix;
        igm = invGlobalMatrix;
    } else {
        gm = ownerNode->computeGlobalMatrix(timerate);
        igm = Matrix4::inverted(gm, nullptr);
    }

    itgm = Matrix4::transposed(igm);
    
    auto* cls = mesh->clusters[isect.clusterId].get();
    const auto& tri = cls->triangles[isect.triangleId];

    auto attrA = cls->attributesAt(tri.a);
    auto attrB = cls->attributesAt(tri.b);
    auto attrC = cls->attributesAt(tri.c);

    RTFloat wa = 1.0 - isect.vcb - isect.vcc;
    RTFloat wb = isect.vcb;
    RTFloat wc = isect.vcc;

    odetail->barycentricCoord.set(wa, wb, wc);
    odetail->vertexAttributes[0] = attrA;
    odetail->vertexAttributes[1] = attrB;
    odetail->vertexAttributes[2] = attrC;
    
    odetail->geometryNormal = Matrix4::transformV3(itgm, tri.normal);
    odetail->geometryNormal.normalize();
    
    odetail->shadingNormal = *attrA.normal * wa + *attrB.normal * wb + *attrC.normal * wc;
    odetail->shadingNormal = Matrix4::transformV3(itgm, odetail->shadingNormal);
    odetail->shadingNormal.normalize();

    odetail->shadingTangent = *attrA.tangent * wa + *attrB.tangent * wb + *attrC.tangent * wc;
    Vector3 tmptan = Matrix4::transformV3(gm, odetail->shadingTangent.getXYZ());
    tmptan.normalize();
    odetail->shadingTangent.x = tmptan.x;
    odetail->shadingTangent.y = tmptan.y;
    odetail->shadingTangent.z = tmptan.z;

    odetail->uvCount = cls->attributeCount(Mesh::kUv);
    odetail->colorCount = cls->attributeCount(Mesh::kColor);

    if (odetail->uvCount > 0) {
        odetail->texcoord0 = *attrA.uv0 * wa + *attrB.uv0 * wb + *attrC.uv0 * wc;
    }
    
    odetail->materialId = cls->material->assetId;
}

// SkinMeshStructure
void SkinMeshStructure::initialize(int maxslice) {
    if (ownerNode->animatedFlag == 0) {
        invGlobalMatrix = Matrix4::inverted(ownerNode->initialTransform.globalMatrix, nullptr);
    }
    jointMatrices.resize(skin->jointNodes.size());
    jointInvTransMatrices.resize(skin->jointNodes.size());
    
    auto* mc = new MeshCache(mesh, maxslice);
    cache = std::unique_ptr<MeshCache>(mc);
}

void SkinMeshStructure::clearSlice() {
    globalBounds.clear();
}

void SkinMeshStructure::updateSlice(int sliceId) {
    // make joint matrix table
    for (size_t ijoint = 0; ijoint < skin->jointNodes.size(); ijoint++) {
        auto* jnode = skin->jointNodes[ijoint];
        auto& ibm = skin->inverseBindMatrices[ijoint];
        jointMatrices[ijoint] = ownerNode->currentInverseGlobal * jnode->currentTransform.globalMatrix * ibm;
        jointInvTransMatrices[ijoint] = Matrix4::transposed(Matrix4::inverted(jointMatrices[ijoint], nullptr));
    }

    cache->createSkinDeformed(sliceId, ownerNode->currentTransform.globalMatrix, jointMatrices, jointInvTransMatrices);
    
    for(auto ite = cache->clusterCaches.begin(); ite != cache->clusterCaches.end(); ++ite) {
        auto* clstr = ite->get();
        globalBounds.expand(clstr->wholeBounds);
    }
}

void SkinMeshStructure::updateFinished() {
    cache->updateBVH();
}

RTFloat SkinMeshStructure::intersection(const Ray& ray, RTFloat nearhit, RTFloat farhit, RTTimeType timerate, MeshIntersection* oisect) const {
    if(!globalBounds.isIntersect(ray, nearhit, farhit)) {
        return -1.0;
    }
    
    // TODO
    RTFloat ret = cache->intersection(ray, nearhit, farhit, timerate, oisect);
    
    return ret;
}

void SkinMeshStructure::intersectionDetail(const Ray& ray, RTFloat hitt, RTTimeType timerate, const MeshIntersection& isect, IntersectionDetail* odetail) const {
    auto* cls = mesh->clusters[isect.clusterId].get();
    auto* ccache = cache->clusterCaches[isect.clusterId].get();

    const auto& otri = cls->triangles[isect.triangleId];

    auto attrA = cls->attributesAt(otri.a);
    auto attrB = cls->attributesAt(otri.b);
    auto attrC = cls->attributesAt(otri.c);

    auto cacheA = ccache->interpolatedCache(otri.a, timerate);
    auto cacheB = ccache->interpolatedCache(otri.b, timerate);
    auto cacheC = ccache->interpolatedCache(otri.c, timerate);

    Mesh::Triangle ctri;

    ctri.initialize(cacheA.vertex, cacheB.vertex, cacheC.vertex);

    RTFloat wa = 1.0 - isect.vcb - isect.vcc;
    RTFloat wb = isect.vcb;
    RTFloat wc = isect.vcc;

    odetail->barycentricCoord.set(wa, wb, wc);
    odetail->vertexAttributes[0] = attrA;
    odetail->vertexAttributes[1] = attrB;
    odetail->vertexAttributes[2] = attrC;

    odetail->geometryNormal = ctri.normal;

    odetail->shadingNormal = cacheA.normal * wa + cacheA.normal * wb + cacheA.normal * wc;
    odetail->shadingNormal.normalize();

    odetail->shadingTangent = cacheA.tangent * wa + cacheA.tangent * wb + cacheA.tangent * wc;
    Vector3 tmptan = odetail->shadingTangent.getXYZ();
    tmptan.normalize();
    odetail->shadingTangent.x = tmptan.x;
    odetail->shadingTangent.y = tmptan.y;
    odetail->shadingTangent.z = tmptan.z;

    odetail->uvCount = cls->attributeCount(Mesh::kUv);
    odetail->colorCount = cls->attributeCount(Mesh::kColor);

    if (odetail->uvCount > 0) {
        odetail->texcoord0 = *attrA.uv0 * wa + *attrB.uv0 * wb + *attrC.uv0 * wc;
    }
    
    odetail->materialId = cls->material->assetId;
}
