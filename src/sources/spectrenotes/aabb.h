#ifndef SPECTRENOTES_AABB_H
#define SPECTRENOTES_AABB_H

#include "types.h"
#include "ray.h"

namespace Spectrenotes {

    class AABB {
    public:
        Vector3 min;
        Vector3 max;
        int dataId;
        int subDataId;
        
        AABB();
        AABB(const RTFloat minval, const RTFloat maxval);
        AABB(const Vector3 minvec, const Vector3 maxvec);
        
        void clear();

        Vector3 size() const;
        RTFloat surfaceArea() const;
        Vector3 centroid() const;
        
        void expand(const Vector3 &p);
        void expand(const AABB &aabb);
        
        bool isInside(const Vector3 &p) const;
        bool isIntersect(const Ray &ray, RTFloat tnear, RTFloat tfar) const;
        RTFloat mightIntersectContent(const Ray &ray, RTFloat tfar) const;
        RTFloat intersectDistance(const Ray &ray) const;
        bool testIntersect(const Ray &ray, RTFloat* otmin, RTFloat* otmax) const;
        
        static AABB transformed(const AABB& a, const Matrix4& m);
    };

}
#endif
