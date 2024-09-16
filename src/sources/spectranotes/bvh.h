#ifndef PINKYPI_BVH_H
#define PINKYPI_BVH_H

#include <vector>
#include <memory>
#include <functional>
#include "aabb.h"

namespace PinkyPi {
    
    class BVH {
    private:
        class TreeNode {
        public:
            TreeNode(const AABB* src): source(src), leftNode(nullptr), rightNode(nullptr) {};

            const AABB* source; // source is null: leaf
            AABB bounds;
            TreeNode* leftNode;
            TreeNode* rightNode;

            void reset(const AABB* bnd);
        };
        
        TreeNode* rootNode;
        std::vector<std::unique_ptr<TreeNode> > nodePool;
        size_t usedNodeCount;
        std::vector<TreeNode*> leafNodes;
        
    public:
        //struct TraverseInfo {
        //    PPFloat(*leafHitCallback)(const Ray&, PPFloat, PPFloat, const AABB*, void*);
        //    void* userRef;
        //};

        BVH();
        BVH(int capacity);
        ~BVH();
        
        void clear();
        void appendLeaf(const AABB* bnd);
        void updateAllLeafBounds();
        void build();

        typedef std::function<PPFloat(const Ray&, PPFloat, PPFloat, const AABB*)> HitCallback;

        // PPFloat intersect(const Ray& ray, PPFloat tnear, PPFloat tfar, const TraverseInfo* tinfo) const;
        PPFloat intersect(const Ray& ray, PPFloat tnear, PPFloat tfar, HitCallback hitfunc) const;
 

    private:
        TreeNode* allocateTreeNode(const AABB* bnd);
        TreeNode* buildTree(TreeNode** childnodes, int numchild, int depth);
        //PPFloat traverseIntersect(const TreeNode* node, const Ray& ray, PPFloat tnear, PPFloat tfar, const TraverseInfo* tinfo) const;
        PPFloat traverseIntersect(const TreeNode* node, const Ray& ray, PPFloat tnear, PPFloat tfar, HitCallback hitfunc) const;

        static int compareTreeNodeX(const void* a, const void* b);
        static int compareTreeNodeY(const void* a, const void* b);
        static int compareTreeNodeZ(const void* a, const void* b);
    };
}


#endif
