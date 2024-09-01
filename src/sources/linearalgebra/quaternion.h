#ifndef LINEARALGEBRA_QUATERNION_H
#define LINEARALGEBRA_QUATERNION_H

#include <cstdio>
#include <cmath>
#include <algorithm>

#include "constants.h"
#include "vector3.h"
#include "matrix4.h"

namespace linearalgebra {
    
    template<typename FPType> union Quaternion {
        struct {FPType x, y, z, w;};
        struct {FPType i, j, k, s;};
        FPType q[4];
        
        // constructors
        Quaternion(): x(0.0), y(0.0), z(0.0), w(1.0) {}
        Quaternion(const FPType ix, const FPType iy, const FPType iz, const FPType iw): x(ix), y(iy), z(iz), w(iw) {}
        
        // self operation
        inline void set(const FPType ix, const FPType iy, const FPType iz, const FPType iw);
        inline void set(const FPType iv[4]);
        
        inline FPType norm(void);

        inline bool normalize(void);
        inline void inverse(void);
        inline void conjugate(void);
        
        inline Matrix4<FPType> getMatrix(void) const;
        
        inline bool hasRotation(void) const;
        inline Vector3<FPType> rotate(const Vector3<FPType> v);
        
        // utility
        static inline Quaternion makeRotation(const FPType rad, const FPType ax, const FPType ay, const FPType az);
        
        static void sprint(char *buf, const Quaternion q);
        
        static inline Quaternion normalized(const Quaternion q);
        static inline Quaternion inversed(const Quaternion q);
        static inline Quaternion conjugated(const Quaternion q);
        
        // 2 quaternion operations
        static inline Quaternion lerp(const Quaternion q0, const Quaternion q1, const FPType t);
        static inline Quaternion slerp(const Quaternion q0, const Quaternion q1, const FPType t);
        
        // operators
        inline Quaternion operator+(const Quaternion &b) const;
        inline Quaternion operator-(const Quaternion &b) const;
        inline Quaternion operator*(const Quaternion &b) const;
        inline Quaternion operator/(const Quaternion &b) const;
        inline Quaternion operator*(const FPType s) const;
        inline Quaternion operator/(const FPType s) const;
        
        inline Quaternion operator+=(const Quaternion &b);
        inline Quaternion operator-=(const Quaternion &b);
    };

    template<typename FPType> inline const Quaternion<FPType> operator*(const FPType s, const Quaternion<FPType> &q);
    template<typename FPType> inline const Quaternion<FPType> operator/(const FPType s, const Quaternion<FPType> &q);
}
// implementation
#include "quaternion.inl"

#endif
