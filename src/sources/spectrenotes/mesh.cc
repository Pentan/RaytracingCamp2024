//
//  mesh.cpp
//  Spectrenotes
//
//  Created by SatoruNAKAJIMA on 2019/08/16.
//

#include "node.h"
#include "mesh.h"
#include "bvh.h"
#include "types.h"

using namespace Spectrenotes;

Mesh::Cluster::Cluster(int numverts, int numtris, const std::map<AttributeId, int>& attrdesc):
    material(nullptr)
{
    vertices.resize(numverts);
    triangles.resize(numtris);

    const size_t attrSizeTbl[] = {
        sizeof(Vector3),    // kNormal,
        sizeof(Vector4),    // kTangent,
        sizeof(Vector3),    // kUv,
        sizeof(Vector4),    // kColor,
        sizeof(IntVec4),    // kJoints,
        sizeof(Vector4),    // kWeights
    };

    attributeDataSize = 0;
    memset(&attributeCounts, 0, sizeof(attributeCounts));

    for (auto kv : attrdesc) {
        attributeCounts[kv.first] = kv.second;
        attributeDataSize += attrSizeTbl[kv.second];
    }

    attributeBuffer.resize(numverts * attributeDataSize);

    attributeOffsets[0] = 0;
    for (int i = 1; i < kNumAttrs; i++) {
        int k = i - 1;
        attributeOffsets[i] = attributeOffsets[k] + attrSizeTbl[k] * attributeCounts[k];
    }
}

Mesh::Cluster::~Cluster() {
}

Attributes Mesh::Cluster::attributesAt(int i) {
    Attributes attrs;
    unsigned char* data = attributeBuffer.data() + attributeDataSize * i;

    attrs.normal = reinterpret_cast<Vector3*>(data + attributeOffsets[kNormal]);
    attrs.tangent = reinterpret_cast<Vector4*>(data + attributeOffsets[kTangent]);
    attrs.uv0 = reinterpret_cast<Vector3*>(data + attributeOffsets[kUv]);
    attrs.color0 = reinterpret_cast<Vector4*>(data + attributeOffsets[kColor]);
    attrs.joints0 = reinterpret_cast<IntVec4*>(data + attributeOffsets[kJoints]);
    attrs.weights0 = reinterpret_cast<Vector4*>(data + attributeOffsets[kNormal]);

    return attrs;
}

int Mesh::Cluster::attributeCount(AttributeId i) const {
    return static_cast<int>(attributeCounts[i]);
}


void Mesh::Triangle::initialize(const Vector3& va, const Vector3& vb, const Vector3& vc) {
    pa = va;
    edgeab = vb - va;
    edgeac = vc - va;
    Vector3 n = Vector3::cross(edgeab, edgeac);
    RTFloat nl = n.length();
    normal = n / std::max(1e-8, nl);
    area = nl;
    bound.clear();
    bound.expand(va);
    bound.expand(vb);
    bound.expand(vc);
}

RTFloat Mesh::Triangle::intersection(const Ray& ray, RTFloat nearhit, RTFloat farhit, RTFloat *obb, RTFloat *obc) const
{
    Vector3 r = ray.origin - pa;
    Vector3 u = Vector3::cross(ray.direction, edgeac);
    Vector3 v = Vector3::cross(r, edgeab);
    
    RTFloat div = 1.0 / Vector3::dot(u, edgeab);
    RTFloat t = Vector3::dot(v, edgeac) * div;
    RTFloat b = Vector3::dot(u, r) * div;
    RTFloat c = Vector3::dot(v, ray.direction) * div;
    
    if((b < 0.0) || (c < 0.0) || (b + c > 1.0) || (t < nearhit) || (t > farhit)) {
        return -1.0;
    }
    
    if(obb != nullptr) { *obb = b; }
    if(obc != nullptr) { *obc = c; }
    return t;
}

Mesh::Mesh()
{
}

Mesh::~Mesh() {
}

void Mesh::setGlobalTransform(const Matrix4 &m) {
    globalTransform = m;
    invGlobalTransform = Matrix4::inverted(m, nullptr);
    invTransGlobalTransform = invGlobalTransform;
    invTransGlobalTransform.transpose();
}

void Mesh::preprocess() {
    
    bounds.clear();
    triangleBVH = std::unique_ptr<BVH>(new BVH(totalTriangles));
    auto* bvh = triangleBVH.get();
    
    // int triangles
    RTFloat meshArea = 0.0;
    for(int icl = 0; icl < clusters.size(); icl++) {
        auto c = clusters[icl].get();
        c->bounds.clear();
        
        RTFloat clusterArea = 0.0;
        for(int itri = 0; itri < c->triangles.size(); itri++) {
            Triangle& tri = c->triangles[itri];
            
            const Vector3& va = c->vertices.at(tri.a);
            const Vector3& vb = c->vertices.at(tri.b);
            const Vector3& vc = c->vertices.at(tri.c);
            
            tri.initialize(va, vb, vc);
            tri.sampleBorder = clusterArea + tri.area;
            tri.bound.dataId = icl;
            tri.bound.subDataId = itri;
            
            c->bounds.expand(tri.bound);
            clusterArea += tri.area;

            bvh->appendLeaf(&tri.bound);
        }
        c->area = clusterArea;
        bounds.expand(c->bounds);
        meshArea += c->area;
    }

    bvh->build();
}

RTFloat Mesh::intersection(const Ray& ray, RTFloat nearhit, RTFloat farhit, MeshIntersection* oisect) const
{
    if(!bounds.isIntersect(ray, nearhit, farhit)) {
        return -1.0;
    }
    
    RTFloat mint = -1.0;
    int triId = -1;
    int clusterId = -1;
    RTFloat vcb = 0.0;
    RTFloat vcc = 0.0;
    RTFloat fart = farhit;

#if 0
    // blute force -----
    int numCls = static_cast<int>(clusters.size());
    for(int icls = 0; icls < numCls; icls++) {
        const Cluster *cls = clusters[icls].get();
        if(!cls->bounds.isIntersect(ray, nearhit, fart)) {
            continue;
        }
        
        int numTris = static_cast<int>(cls->triangles.size());
        for(int itri = 0; itri < numTris; itri++) {
            const Triangle& tri = cls->triangles[itri];
            RTFloat tb = 0.0;
            RTFloat tc = 0.0;
            RTFloat thit = tri.intersection(ray, nearhit, fart, &tb, &tc);
            if(thit > 0.0) {
                if(mint > thit || mint < 0.0) {
                    mint = thit;
                    fart = thit;
                    triId = itri;
                    clusterId = icls;
                    vcb = tb;
                    vcc = tc;
                }
            }
        }
    }
    //-----
#else
    struct {
        RTFloat mint;
        int clusterId;
        int triId;
        RTFloat vb, vc;
    } hitInfo;

    memset(&hitInfo, 0, sizeof(hitInfo));
    hitInfo.mint = -1.0;
    RTFloat mintb = triangleBVH->intersect(ray, nearhit, farhit, [this, &hitInfo](const Ray& ray, RTFloat neart, RTFloat fart, const AABB* tribnd) {
        const int clsId = tribnd->dataId;
        const int triId = tribnd->subDataId;
        const Triangle& tri = clusters[clsId]->triangles[triId];
        RTFloat b;
        RTFloat c;
        RTFloat t = tri.intersection(ray, neart, fart, &b, &c);
        if (t > 0.0) {
            if (hitInfo.mint > t || hitInfo.mint < 0.0) {
                hitInfo.mint = t;
                hitInfo.triId = triId;
                hitInfo.clusterId = clsId;
                hitInfo.vb = b;
                hitInfo.vc = c;
            }
        }
        return t;
    });
    
//    if(mint != mintb) {
//        printf("miss hit (%lf,<%d,%d>):(%lf,<%d,%d>)!!!!!\n",
//               mint, clusterId, triId,
//               hitInfo.mint, hitInfo.clusterId, hitInfo.triId);
//    }

    mint = hitInfo.mint;
    clusterId = hitInfo.clusterId;
    triId = hitInfo.triId;
    vcb = hitInfo.vb;
    vcc = hitInfo.vc;

#endif

    if(oisect != nullptr) {
        oisect->meshId = assetId;
        oisect->clusterId = clusterId;
        oisect->triangleId = triId;
        oisect->vcb = vcb;
        oisect->vcc = vcc;
    }
    
    return mint;
}

//
MeshCache::ClusterCache::ClusterCache(Mesh::Cluster* src, int numslice) :
sourceCluster(src)
{
    sliceArea.resize(numslice);
    sliceBounds.resize(numslice);
    cachedVertices.resize(numslice);
    
    wholeTriBounds.resize(sourceCluster->triangles.size());
    for (size_t i = 0; i < wholeTriBounds.size(); i++) {
        const auto& tri = sourceCluster->triangles[i];
        wholeTriBounds[i] = tri.bound;
        wholeTriBounds[i].clear();
    }
    wholeBounds.clear();

    for (int islc = 0; islc < numslice; islc++) {
        sliceArea[islc] = 0.0;
        sliceBounds[islc].clear();
        auto& cachevert = cachedVertices[islc];
        cachevert.resize(sourceCluster->vertices.size());

        // initial values
        for (size_t i = 0; i < sourceCluster->vertices.size(); i++) {
            Attributes attrs = sourceCluster->attributesAt(static_cast<int>(i));
            cachevert[i].vertex = sourceCluster->vertices[i];
            cachevert[i].normal = *attrs.normal;
            cachevert[i].tangent = *attrs.tangent;
            sliceBounds[islc].expand(cachevert[i].vertex);
        }

        expandWholeTriangleBounds(islc);
        wholeBounds.expand(sliceBounds[islc]);
    }
}

void MeshCache::ClusterCache::clearWholeSliceData() {
    wholeBounds.clear();
    for (auto ite = wholeTriBounds.begin(); ite != wholeTriBounds.end(); ++ite) {
        ite->clear();
    }
}

void MeshCache::ClusterCache::expandWholeTriangleBounds(int sliceid) {
    Mesh::Triangle tmptri;
    const auto& cachedvert = cachedVertices[sliceid];
    auto& area = sliceArea[sliceid];

    area = 0.0;
    for (size_t i = 0; i < sourceCluster->triangles.size(); i++) {
        auto& tri = sourceCluster->triangles[i];
        tmptri.initialize(cachedvert[tri.a].vertex, cachedvert[tri.b].vertex, cachedvert[tri.c].vertex);
        wholeTriBounds[i].expand(tmptri.bound);
        area += tmptri.area;
    }
}

void MeshCache::ClusterCache::createTransformed(int sliceid, const Matrix4& m) {
    auto& cachedvert = cachedVertices[sliceid];
    auto& bnd = sliceBounds[sliceid];
    Matrix4 itm = Matrix4::transposed(Matrix4::inverted(m, nullptr));
    bnd.clear();
    for(size_t i = 0; i < sourceCluster->vertices.size(); i++) {
        Attributes attrs = sourceCluster->attributesAt(static_cast<int>(i));
        cachedvert[i].vertex = Matrix4::transformV3(m, sourceCluster->vertices[i]);
        cachedvert[i].normal = Matrix4::mulV3(itm, *attrs.normal);
        Vector3 tmpv3(attrs.tangent->x, attrs.tangent->y, attrs.tangent->z);
        tmpv3 = Matrix4::mulV3(m, tmpv3);
        cachedvert[i].tangent.set(tmpv3.x, tmpv3.y, tmpv3.z, attrs.tangent->w);
        bnd.expand(cachedvert[i].vertex);
    }

    expandWholeTriangleBounds(sliceid);
    wholeBounds.expand(bnd);
}

void MeshCache::ClusterCache::createSkinDeformed(int sliceid, const Matrix4& m, const std::vector<Matrix4>& mplt, const std::vector<Matrix4>& itmplt) {
    Matrix4 itm = Matrix4::transposed(Matrix4::inverted(m, nullptr));
    auto& cachedvert = cachedVertices[sliceid];
    auto& bnd = sliceBounds[sliceid];
    bnd.clear();
    const int numinfl = sourceCluster->attributeCount(Mesh::AttributeId::kWeights);
    for (size_t i = 0; i < sourceCluster->vertices.size(); i++) {
        Attributes attrs = sourceCluster->attributesAt(static_cast<int>(i));
        RTFloat totalWeights = 0.0;

        const auto& sv = sourceCluster->vertices[i];
        const auto& sn = *attrs.normal;
        const Vector3 st(attrs.tangent->x, attrs.tangent->y, attrs.tangent->z);

        Vector3 tmpv;
        Vector3 tmpn;
        Vector3 tmpt;

        for (int iinfl = 0; iinfl < numinfl; iinfl++) {
            const auto& w4 = attrs.weights0[iinfl];
            const auto& j4 = attrs.joints0[iinfl];
            
            tmpv += Matrix4::transformV3(mplt[j4.x], sv) * w4.x;
            tmpv += Matrix4::transformV3(mplt[j4.y], sv) * w4.y;
            tmpv += Matrix4::transformV3(mplt[j4.z], sv) * w4.z;
            tmpv += Matrix4::transformV3(mplt[j4.w], sv) * w4.w;

            tmpn += Matrix4::mulV3(itmplt[j4.x], sn) * w4.x;
            tmpn += Matrix4::mulV3(itmplt[j4.y], sn) * w4.y;
            tmpn += Matrix4::mulV3(itmplt[j4.z], sn) * w4.z;
            tmpn += Matrix4::mulV3(itmplt[j4.w], sn) * w4.w;

            tmpt += Matrix4::mulV3(mplt[j4.x], st) * w4.x;
            tmpt += Matrix4::mulV3(mplt[j4.y], st) * w4.y;
            tmpt += Matrix4::mulV3(mplt[j4.z], st) * w4.z;
            tmpt += Matrix4::mulV3(mplt[j4.w], st) * w4.w;

            totalWeights += w4.x + w4.y + w4.z + w4.w;
        }

        totalWeights = 1.0 / totalWeights;
        cachedvert[i].vertex = Matrix4::transformV3(m, tmpv * totalWeights);
        cachedvert[i].normal = Vector3::normalized(Matrix4::mulV3(itm, tmpn * totalWeights));
        tmpt = Vector3::normalized(Matrix4::mulV3(m, tmpt * totalWeights));
        cachedvert[i].tangent.set(tmpt.x, tmpt.y, tmpt.z, attrs.tangent->w);

        bnd.expand(cachedvert[i].vertex);
    }

    expandWholeTriangleBounds(sliceid);
    wholeBounds.expand(bnd);
}

MeshCache::CachedAttribute MeshCache::ClusterCache::interpolatedCache(int vid, RTTimeType timerate) const {
    int slicelast = static_cast<int>(cachedVertices.size() - 1);
    RTTimeType slicerate = timerate * slicelast;
    RTTimeType t = slicerate - std::floor(slicerate);
    int i0 = static_cast<int>(std::floor(slicerate));
    int i1 = std::min(i0 + 1, slicelast);

    CachedAttribute ret;
    const auto& sv0 = cachedVertices[i0][vid];
    const auto& sv1 = cachedVertices[i1][vid];

    ret.vertex = Vector3::lerp(sv0.vertex, sv1.vertex, t);
    ret.normal = Vector3::normalized(Vector3::lerp(sv0.normal, sv1.normal, t));
    ret.tangent = Vector4::lerp(sv0.tangent, sv1.tangent, t);
    auto tmptan = Vector3::normalized(ret.tangent.getXYZ());
    ret.tangent.set(tmptan, ret.tangent.w);

    return ret;
}

MeshCache::MeshCache(Mesh* m, int numslice) : mesh(m), sliceCount(numslice) {
    skinedBVH = std::unique_ptr<BVH>(new BVH(mesh->totalTriangles));
    auto* bvh = skinedBVH.get();

    size_t numclstr = mesh->clusters.size();
    clusterCaches.resize(numclstr);
    for(size_t i = 0; i < numclstr; i++) {
        auto* cc = new ClusterCache(mesh->clusters[i].get(), sliceCount);
        clusterCaches[i] = std::unique_ptr<ClusterCache>(cc);
        
        for (size_t itr = 0; itr < cc->wholeTriBounds.size(); itr++) {
            bvh->appendLeaf(&cc->wholeTriBounds[itr]);
        }
    }
}

void MeshCache::updateBVH() {
    auto* bvh = skinedBVH.get();
    bvh->updateAllLeafBounds();
    bvh->build();
}

RTFloat MeshCache::intersection(const Ray& ray, RTFloat nearhit, RTFloat farhit, RTTimeType timerate, MeshIntersection* oisect) const {

    RTFloat mint = -1.0;
    int triId = -1;
    int clusterId = -1;
    RTFloat vcb = 0.0;
    RTFloat vcc = 0.0;

#if 0
    // blute force -----
    int numCls = static_cast<int>(clusterCaches.size());
    Mesh::Triangle tmptri;
    RTFloat fatt = farhit;
    for(int icls = 0; icls < numCls; icls++) {
        const auto *ccache = clusterCaches[icls].get();
        //if(!ccache->wholeBounds.isIntersect(ray, nearhit, fatt)) {
        //    continue;
        //}
        
        const auto *cls = mesh->clusters[icls].get();
        int numTris = static_cast<int>(cls->triangles.size());
        for(int itri = 0; itri < numTris; itri++) {
            const auto& tri = cls->triangles[itri];

            if (!ccache->wholeTriBounds[itri].isIntersect(ray, nearhit, fatt)) continue;
            
            auto cva = ccache->interpolatedCache(tri.a, timerate);
            auto cvb = ccache->interpolatedCache(tri.b, timerate);
            auto cvc = ccache->interpolatedCache(tri.c, timerate);
            
            tmptri.initialize(cva.vertex, cvb.vertex, cvc.vertex);
            RTFloat b;
            RTFloat c;
            RTFloat thit = tmptri.intersection(ray, nearhit, fatt, &b, &c);
            if(thit > 0.0) {
                if(mint > thit || mint < 0.0) {
                    mint = thit;
                    fatt = thit;
                    triId = itri;
                    clusterId = icls;
                    vcb = b;
                    vcc = c;
                }
            }
        }
    }

    //-----
#else
    // BVH
    struct {
        RTFloat mint;
        int clusterId;
        int triId;
        RTFloat vb, vc;
    } hitInfo;

    hitInfo.mint = -1.0;
    mint = skinedBVH->intersect(ray, nearhit, farhit, [this, &hitInfo, timerate](const Ray& ray, RTFloat neart, RTFloat fart, const AABB* tribnd) {
        const int clsId = tribnd->dataId;
        const int triId = tribnd->subDataId;
        const auto* cls = mesh->clusters[clsId].get();
        const auto& tri = cls->triangles[triId];
        const auto* ccache = clusterCaches[clsId].get();

        auto cva = ccache->interpolatedCache(tri.a, timerate);
        auto cvb = ccache->interpolatedCache(tri.b, timerate);
        auto cvc = ccache->interpolatedCache(tri.c, timerate);

        Mesh::Triangle tmptri;
        tmptri.initialize(cva.vertex, cvb.vertex, cvc.vertex);

        RTFloat b;
        RTFloat c;
        RTFloat t = tmptri.intersection(ray, neart, fart, &b, &c);
        if (t > 0.0) {
            if (hitInfo.mint > t || hitInfo.mint < 0.0) {
                hitInfo.mint = t;
                hitInfo.triId = triId;
                hitInfo.clusterId = clsId;
                hitInfo.vb = b;
                hitInfo.vc = c;
            }
        }
        return t;
    });

    clusterId = hitInfo.clusterId;
    triId = hitInfo.triId;
    vcb = hitInfo.vb;
    vcc = hitInfo.vc;
#endif

    if(oisect != nullptr) {
        oisect->meshId = mesh->assetId;
        oisect->clusterId = clusterId;
        oisect->triangleId = triId;
        oisect->vcb = vcb;
        oisect->vcc = vcc;
    }
    
    return mint;
}

void Mesh::triangleAttributes(int clusterId, int triangleId, Attributes* oattr3) const {
    auto* cls = clusters[clusterId].get();
    auto& tri = cls->triangles[triangleId];
    oattr3[0] = cls->attributesAt(tri.a);
    oattr3[1] = cls->attributesAt(tri.b);
    oattr3[2] = cls->attributesAt(tri.c);
}
