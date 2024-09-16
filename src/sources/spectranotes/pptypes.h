#ifndef PINKYPI_TYPES_H
#define PINKYPI_TYPES_H

#include <linearalgebra/linearalgebra.h>

namespace PinkyPi {

    // math
    typedef double PPFloat;

    const PPFloat kPI = 3.14159265358979323846;
    const PPFloat kINF = 1e128;
    const PPFloat kEPS = 1e-8;
    const PPFloat kRayOffset = 1e-4;
    const PPFloat kFarAway = 1e16;
    #define kSmallOffset    kEPS

    typedef linearalgebra::Vector3<PPFloat> Vector3;
    typedef linearalgebra::Vector4<PPFloat> Vector4;
    typedef linearalgebra::Matrix4<PPFloat> Matrix4;
    typedef linearalgebra::Quaternion<PPFloat> Quaterion;

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
    typedef double PPColorType;
    typedef linearalgebra::Vector3<PPColorType> Color;
    
    // time
    typedef double PPTimeType;

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
