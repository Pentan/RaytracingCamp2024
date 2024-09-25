#ifndef SPECTRENOTES_TYPES_H
#define SPECTRENOTES_TYPES_H

#include <linearalgebra/linearalgebra.h>

namespace Spectrenotes {

    // math
    typedef double RTFloat;

    const RTFloat kPI = 3.14159265358979323846;
    const RTFloat kINF = 1e128;
    const RTFloat kEPS = 1e-8;
    const RTFloat kRayOffset = 1e-4;
    const RTFloat kFarAway = 1e16;
    #define kSmallOffset    kEPS

    typedef linearalgebra::Vector3<RTFloat> Vector3;
    typedef linearalgebra::Vector4<RTFloat> Vector4;
    typedef linearalgebra::Matrix4<RTFloat> Matrix4;
    typedef linearalgebra::Quaternion<RTFloat> Quaterion;

    union IntVec3 {
        struct { int x, y, z; };
        struct { int i, j, k; };
        struct { int i0, i1, i2; };
        int v[3];
    };

    union IntVec4 {
        struct { int x, y, z, w; };
        struct { int i, j, k, l; };
        struct { int i0, i1, i2, i3; };
        int v[4];
    };

    // color
    typedef double RTColorType;
    typedef linearalgebra::Vector3<RTColorType> Color;
    
    // time
    typedef double RTTimeType;

    // vertex attribute
    struct Attributes {
        Vector3* normal;
        Vector4* tangent;
        Vector3* uv0;
        Vector4* color0;
        IntVec4* joints0;
        Vector4* weights0;
    };

    // surface
    struct SurfaceInfo {
        Vector3 position;
        Vector3 geometryNormal;
        Vector3 shadingNormal;
        Vector3* uv0;
    };
}

#endif
