#include "bvh.h"

using namespace Petals;

/////
void BVH::TreeNode::reset(const AABB* bnd) {
    source = bnd;
    bounds.clear();
    leftNode = nullptr;
    rightNode = nullptr;
}

/////
BVH::BVH():
    rootNode(nullptr),
    usedNodeCount(0)
{
}

BVH::BVH(int capacity):
    BVH()
{
    leafNodes.reserve(capacity);
    for(int i = 0; i < capacity * 2; i++) {
        auto node = new TreeNode(nullptr);
        nodePool.push_back(std::unique_ptr<TreeNode>(node));
    }
}

BVH::~BVH()
{
}


void BVH::clear() {
    leafNodes.clear();
    usedNodeCount = 0;
}

void BVH::appendLeaf(const AABB* bnd) {
    auto node = allocateTreeNode(bnd);
    node->bounds = *bnd;
    leafNodes.push_back(node);
}

void BVH::updateAllLeafBounds() {
    for(auto ite = leafNodes.begin(); ite != leafNodes.end(); ++ite) {
        auto* node = *ite;
        node->reset(node->source);
        node->bounds = *node->source;
    }
}

void BVH::build()
{
    usedNodeCount = leafNodes.size();
    rootNode = buildTree(leafNodes.data(), static_cast<int>(leafNodes.size()), 0);
}

BVH::TreeNode* BVH::allocateTreeNode(const AABB* bnd) {
    TreeNode* node;
    if (usedNodeCount < nodePool.size()) {
        node = nodePool[usedNodeCount].get();
        node->reset(bnd);
    } else {
        node = new TreeNode(bnd);
        nodePool.push_back(std::unique_ptr<TreeNode>(node));
    }
    usedNodeCount += 1;
    return node;
}

BVH::TreeNode* BVH::buildTree(TreeNode** childnodes, int numchild, int depth)
{
    if(numchild < 2) {
        // leaf
        return childnodes[0];
    } else {
        auto curnode = allocateTreeNode(nullptr);
        
        for(int i = 0; i < numchild; i++) {
            curnode->bounds.expand(childnodes[i]->bounds);
        }
        
        Vector3 boundSize = curnode->bounds.size();
        int splitAxis = (std::abs(boundSize.x) > std::abs(boundSize.y))? 0 : 1;
        splitAxis = (std::abs(boundSize.v[splitAxis]) > std::abs(boundSize.z))? splitAxis : 2;
        
        int (*funcs[])(const void*, const void*) = {
            compareTreeNodeX,
            compareTreeNodeY,
            compareTreeNodeZ
        };

        std::qsort(childnodes, numchild, sizeof(childnodes[0]), funcs[splitAxis]);

        // center split
        int numleft = numchild / 2;
        int numright = numchild - numleft;

        curnode->leftNode = buildTree(childnodes, numleft, depth + 1);
        curnode->rightNode = buildTree(childnodes + numleft, numright, depth + 1);

        return curnode;
    }
}

RTFloat BVH::intersect(const Ray& ray, RTFloat tnear, RTFloat tfar, HitCallback hitfunc) const {
    if (!rootNode->bounds.isIntersect(ray, tnear, tfar)) {
        return -1.0;
    }
    return traverseIntersect(rootNode, ray, tnear, tfar, hitfunc);
}

RTFloat BVH::traverseIntersect(const TreeNode* node, const Ray& ray, RTFloat tnear, RTFloat tfar, HitCallback hitfunc) const {
    if (node->source != nullptr) {
        return hitfunc(ray, tnear, tfar, node->source);
    }
    else {
//        RTFloat t;
        RTFloat rett = -1.0;
        RTFloat tl = node->leftNode->bounds.mightIntersectContent(ray, tfar);
        RTFloat tr = node->rightNode->bounds.mightIntersectContent(ray, tfar);
        
        if (tl >= 0.0) {
            RTFloat t = traverseIntersect(node->leftNode, ray, tnear, tfar, hitfunc);
            if (t >= tnear && t <= tfar) {
                tfar = t;
                rett = t;
            }
        }
        
        if (tr >= 0.0) {
            RTFloat t = traverseIntersect(node->rightNode, ray, tnear, tfar, hitfunc);
            if (t >= tnear && t <= tfar) {
                rett = (rett < 0.0) ? t : std::min(rett, t);
            }
        }
        return rett;
    }
}

//RTFloat BVH::intersect(const Ray& ray, RTFloat tnear, RTFloat tfar, const TraverseInfo* tinfo) const {
//    if (!rootNode->bounds.isIntersect(ray, tnear, tfar)) {
//        return -1.0;
//    }
//    return traverseIntersect(rootNode, ray, tnear, tfar, tinfo);
//}
//
//RTFloat BVH::traverseIntersect(const TreeNode* node, const Ray& ray, RTFloat tnear, RTFloat tfar, const TraverseInfo* tinfo) const {
//    if (node->source != nullptr) {
//        return tinfo->leafHitCallback(ray, tnear, tfar, node->source, tinfo->userRef);
//    } else {
//        RTFloat t;
//        RTFloat rett = -1.0;
//        if(node->leftNode->bounds.isIntersect(ray, tnear, tfar)) {
//            t = traverseIntersect(node->leftNode, ray, tnear, tfar, tinfo);
//            if(t > tnear && t < tfar) {
//                tfar = t;
//                rett = t;
//            }
//        }
//
//        t = node->rightNode->bounds.intersectDistance(ray);
//        if (t > tnear && t < tfar) {
//            t = traverseIntersect(node->leftNode, ray, tnear, tfar, tinfo);
//            if (t > tnear && t < tfar) {
//                rett = (rett < 0.0) ? t : std::min(rett, t);
//            }
//        }
//        return rett;
//    }
//}

int BVH::compareTreeNodeX(const void* p0, const void* p1) {
    const TreeNode* n0 = reinterpret_cast<const TreeNode*>(p0);
    const TreeNode* n1 = reinterpret_cast<const TreeNode*>(p1);
    RTFloat a = n0->bounds.centroid().x;
    RTFloat b = n1->bounds.centroid().x;
    return (a == b) ? 0 : ((a < b) ? -1 : 1);
}

int BVH::compareTreeNodeY(const void* p0, const void* p1) {
    const TreeNode* n0 = reinterpret_cast<const TreeNode*>(p0);
    const TreeNode* n1 = reinterpret_cast<const TreeNode*>(p1);
    RTFloat a = n0->bounds.centroid().y;
    RTFloat b = n1->bounds.centroid().y;
    return (a == b) ? 0 : ((a < b) ? -1 : 1);
}

int BVH::compareTreeNodeZ(const void* p0, const void* p1) {
    const TreeNode* n0 = reinterpret_cast<const TreeNode*>(p0);
    const TreeNode* n1 = reinterpret_cast<const TreeNode*>(p1);
    RTFloat a = n0->bounds.centroid().z;
    RTFloat b = n1->bounds.centroid().z;
    return (a == b) ? 0 : ((a < b) ? -1 : 1);
}
