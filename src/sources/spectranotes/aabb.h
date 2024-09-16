#ifndef PINKYPI_AABB_H
#define PINKYPI_AABB_H

#include "pptypes.h"
#include "ray.h"

namespace PinkyPi {

    class AABB {
    public:
        Vector3 min;
        Vector3 max;
        int dataId;
        int subDataId;
        
        AABB();
        AABB(const PPFloat minval, const PPFloat maxval);
        AABB(const Vector3 minvec, const Vector3 maxvec);
        
        void clear();

        Vector3 size() const;
        PPFloat surfaceArea() const;
        Vector3 centroid() const;
        
        void expand(const Vector3 &p);
        void expand(const AABB &aabb);
        
        bool isInside(const Vector3 &p) const;
        bool isIntersect(const Ray &ray, PPFloat tnear, PPFloat tfar) const;
        PPFloat mightIntersectContent(const Ray &ray, PPFloat tfar) const;
        PPFloat intersectDistance(const Ray &ray) const;
        bool testIntersect(const Ray &ray, PPFloat* otmin, PPFloat* otmax) const;
        
        static AABB transformed(const AABB& a, const Matrix4& m);
    };

}
#endif
