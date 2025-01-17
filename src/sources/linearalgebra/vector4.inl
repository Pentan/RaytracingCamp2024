#ifndef LINEARALGEBRA_VECTOR4_INL
#define LINEARALGEBRA_VECTOR4_INL

namespace linearalgebra {

    // self operation
    template<typename FPType>
    inline void Vector4<FPType>::set(const FPType ix, const FPType iy, const FPType iz, const FPType iw) {
        x = ix;
        y = iy;
        z = iz;
        w = iw;
    }
    template<typename FPType>
    inline void Vector4<FPType>::set(const FPType iv[4]) {
        x = iv[0];
        y = iv[1];
        z = iv[2];
        w = iv[3];
    }
    template<typename FPType>
    inline void Vector4<FPType>::set(const Vector3<FPType> vxyz, const FPType iw) {
        x = vxyz.x;
        y = vxyz.y;
        z = vxyz.z;
        w = iw;
    }

    template<typename FPType>
    inline Vector3<FPType> Vector4<FPType>::getXYZ(void) const {
        return Vector3<FPType>(x, y, z);
    }

    template<typename FPType>
    inline FPType Vector4<FPType>::length(void) const {
        return sqrt(x * x + y * y + z * z + w * w);
    }    
    template<typename FPType>
    inline bool Vector4<FPType>::isZero(void) const {
        return x == 0.0 && y == 0.0 && z == 0.0 && w == 0.0;
    }
    template<typename FPType>
    inline bool Vector4<FPType>::isZero(FPType eps) const {
        return std::abs(x) < eps && std::abs(y) < eps && std::abs(z) < eps && std::abs(w) < eps;
    }

    template<typename FPType>
    inline bool Vector4<FPType>::normalize(void) {
        FPType l = x * x + y * y + z * z + w * w;
        if(l > kEPS) {
            l = 1.0 / sqrt(l);
            x *= l;
            y *= l;
            z *= l;
            w *= l;
            return true;
        }
        return false;
    }
    template<typename FPType>
    inline void Vector4<FPType>::negate(void) {
        x = -x;
        y = -y;
        z = -z;
        w = -w;
    }
    
    template<typename FPType>
    inline FPType Vector4<FPType>::getMaxComponent(int* outcompid) const {
        if(outcompid) {
            *outcompid = (x > y)? kX : kY;
            *outcompid = (v[*outcompid] > z)? *outcompid : kZ;
            *outcompid = (v[*outcompid] > w)? *outcompid : kW;
            return v[*outcompid];
        } else {
            return std::max(x, std::max(y, std::max(z, w)));
        }
    }
    
    template<typename FPType>
    inline FPType Vector4<FPType>::getMinComponent(int* outcompid) const {
        if(outcompid) {
            *outcompid = (x < y)? kX : kY;
            *outcompid = (v[*outcompid] < z)? *outcompid : kZ;
            *outcompid = (v[*outcompid] < w)? *outcompid : kW;
            return v[*outcompid];
        } else {
            return std::min(x, std::min(y, std::min(z, w)));
        }
    }

    // 2 vector
    template<typename FPType>
    inline FPType Vector4<FPType>::distance(const Vector4 v0, const Vector4 v1) {
        Vector4 tmpv = v1 - v0;
        return tmpv.length();
    }
    template<typename FPType>
    inline Vector4<FPType> Vector4<FPType>::normalized(const Vector4 v) {
        Vector4 retv = v;
        retv.normalize();
        return retv;
    }
    template<typename FPType>
    inline Vector4<FPType> Vector4<FPType>::negated(const Vector4 v) {
        return Vector4(-v.x, -v.y, -v.z, -v.w);
    }
    template<typename FPType>
    inline Vector4<FPType> Vector4<FPType>::mul(const Vector4 v0, const Vector4 v1) {
        Vector4 retv;
        retv.x = v0.x * v1.x;
        retv.y = v0.y * v1.y;
        retv.z = v0.z * v1.z;
        retv.w = v0.w * v1.w;
        return retv;
    }
    template<typename FPType>
    inline Vector4<FPType> Vector4<FPType>::div(const Vector4 v0, const Vector4 v1) {
        Vector4 retv;
        retv.x = v0.x / v1.x;
        retv.y = v0.y / v1.y;
        retv.z = v0.z / v1.z;
        retv.w = v0.w / v1.w;
        return retv;
    }
    template<typename FPType> 
    inline FPType Vector4<FPType>::dot(const Vector4 v0, const Vector4 v1) {
        return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z + v0.w * v1.w;
    }

    template<typename FPType> 
    inline Vector4<FPType> Vector4<FPType>::lerp(const Vector4 v0, const Vector4 v1, FPType t) {
        Vector4 retv;
        FPType t0 = 1.0f - t;
        retv.x = v0.x * t0 + v1.x * t;
        retv.y = v0.y * t0 + v1.y * t;
        retv.z = v0.z * t0 + v1.z * t;
        retv.w = v0.w * t0 + v1.w * t;
        return retv;
    }
    template<typename FPType> 
    inline Vector4<FPType> Vector4<FPType>::project(const Vector4 v0, const Vector4 v1) {
        Vector4 nv = Vector4::normalized(v1);
        FPType d = Vector4::dot(v0, nv);
        return nv * d;
    }

    // util
    template<typename FPType> 
    void Vector4<FPType>::sprint(char *buf, const Vector4 v) {
        sprintf(buf, "v4(%.4lf,%.4lf,%.4lf,%.4lf)", (double)v.x, (double)v.y, (double)v.z, (double)v.w);
    }

    // operators
    template<typename FPType> inline Vector4<FPType> Vector4<FPType>::operator+(const Vector4 &v) const {
        return Vector4(x + v.x, y + v.y, z + v.z, w + v.w);
    }
    template<typename FPType> inline Vector4<FPType> Vector4<FPType>::operator-(const Vector4 &v) const {
        return Vector4(x - v.x, y - v.y, z - v.z, w - v.w);
    }
    template<typename FPType> inline Vector4<FPType> Vector4<FPType>::operator*(const FPType s) const {
        return Vector4(x * s, y * s, z * s, w * s);
    }
    template<typename FPType> inline Vector4<FPType> Vector4<FPType>::operator/(const FPType s) const {
        return Vector4(x / s, y / s, z / s, w / s);
    }
    template<typename FPType> inline Vector4<FPType> Vector4<FPType>::operator+=(const Vector4 &b) {
        x += b.x;
        y += b.y;
        z += b.z;
        w += b.w;
        return *this;
    }
    template<typename FPType> inline Vector4<FPType> Vector4<FPType>::operator-=(const Vector4 &b) {
        x -= b.x;
        y -= b.y;
        z -= b.z;
        w -= b.w;
        return *this;
    }

    template<typename FPType> inline const Vector4<FPType> operator*(const FPType s, const Vector4<FPType> &v) {
        return v * s;
    }

    template<typename FPType> inline const Vector4<FPType> operator/(const FPType s, const Vector4<FPType> &v) {
        return v / s;
    }
}

#endif
