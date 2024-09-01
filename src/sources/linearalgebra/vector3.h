#ifndef LINEARALGEBRA_VECTOR3_H
#define LINEARALGEBRA_VECTOR3_H

#include <cstdio>
#include <cmath>
#include <algorithm>
#include "constants.h"

namespace linearalgebra {

    template<typename FPType> union Vector3 {
        struct {FPType x, y, z;};
        struct {FPType r, g, b;};
        struct {FPType s, t, p;};
        FPType v[3];
        
        //
        Vector3(): x(0.0), y(0.0), z(0.0) {}
        Vector3(const FPType n): x(n), y(n), z(n) {}
        Vector3(const FPType ix, const FPType iy, const FPType iz): x(ix), y(iy), z(iz) {}
        
        // self operations
        inline void set(const FPType ix, const FPType iy, const FPType iz);
        inline void set(const FPType iv[3]);
        
        inline FPType length(void) const;
        inline bool isZero(void) const;         // x,y,z == 0.0
        inline bool isZero(FPType eps) const;   // |x,y,z| < eps
        
        inline bool normalize(void);
        inline void negate(void);
        
        inline FPType getMaxComponent(int* outcompid=0) const;
        inline FPType getMinComponent(int* outcompid=0) const;
        
        // 2 vector operations
        static inline FPType distance(const Vector3 v0, const Vector3 v1);
        
        static inline Vector3 normalized(const Vector3 v);
        static inline Vector3 negated(const Vector3 v);
        
        static inline Vector3 mul(const Vector3 v0, const Vector3 v1); // multiple each component
        static inline Vector3 div(const Vector3 v0, const Vector3 v1); // divide each component
        
        static inline FPType dot(const Vector3 v0, const Vector3 v1);
        static inline Vector3 cross(const Vector3 v0, const Vector3 v1);
        
        static inline Vector3 lerp(const Vector3 v0, const Vector3 v1, const FPType t);
        static inline Vector3 project(const Vector3 v0, const Vector3 v1);
        
        // utils
        static void sprint(char *buf, const Vector3 v);
        
        // operators
        inline Vector3 operator+(const Vector3 &b) const;
        inline Vector3 operator-(const Vector3 &b) const;
        inline Vector3 operator*(const FPType s) const;
        inline Vector3 operator/(const FPType s) const;
        
        inline Vector3 operator+=(const Vector3 &b);
        inline Vector3 operator-=(const Vector3 &b);
    };

    template<typename FPType> inline const Vector3<FPType> operator*(const FPType s, const Vector3<FPType> &v);
    template<typename FPType> inline const Vector3<FPType> operator/(const FPType s, const Vector3<FPType> &v);
}

// implementation
#include "vector3.inl"

#endif
