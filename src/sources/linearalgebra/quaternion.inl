#ifndef LINEARALGEBRA_QUATERNION_INL
#define LINEARALGEBRA_QUATERNION_INL

namespace linearalgebra {
    
    template<typename FPType>
    inline void Quaternion<FPType>::set(const FPType ix, const FPType iy, const FPType iz, const FPType iw){
        x = ix;
        y = iy;
        z = iz;
        w = iw;
    }
    
    template<typename FPType>
    inline void Quaternion<FPType>::set(const FPType iv[4]){
        x = iv[0];
        y = iv[1];
        z = iv[2];
        w = iv[3];
    }

    template<typename FPType>
    inline FPType Quaternion<FPType>::norm(void) {
        return sqrt(x * x + y * y + z * z + w * w);
    }
    
    template<typename FPType>
    inline bool Quaternion<FPType>::normalize(void) {
        FPType n = this->norm();
        if(n > kEPS) {
            n = 1.0 / n;
            x *= n;
            y *= n;
            z *= n;
            w *= n;
            return true;
        }
        return false;
    }
    
    template<typename FPType>
    inline void Quaternion<FPType>::inverse(void) {
        FPType n = x * x + y * y + z * z + w * w;
        if (n < kEPS) return;
        n = 1.0 / n;
        x *= -n;
        y *= -n;
        z *= -n;
        w *= n;
    }
    
    template<typename FPType>
    inline void Quaternion<FPType>::conjugate(void) {
        x *= -1.0;
        y *= -1.0;
        z *= -1.0;
    }
    
    template<typename FPType>
    inline Matrix4<FPType> Quaternion<FPType>::getMatrix(void) const {
        Matrix4<FPType> m;
        
        m.m00 = x * x - y * y - z * z + w * w;
        m.m01 = 2.0 * (x * y + z * w);
        m.m02 = 2.0 * (x * z - y * w);
        m.m03 = 0.0;
        
        m.m10 = 2.0 * (x * y - z * w);
        m.m11 = -x * x + y * y - z * z + w * w;
        m.m12 = 2.0 * (y * z + x * w);
        m.m13 = 0.0;
        
        m.m20 = 2.0 * (x * z + y * w);
        m.m21 = 2.0 * (y * z - x * w);
        m.m22 = -x * x - y * y + z * z + w * w;
        m.m23 = 0.0;
        
        m.m30 = 0.0;
        m.m31 = 0.0;
        m.m32 = 0.0;
        m.m33 = 1.0;
        
        return m;
    }
    
    template<typename FPType>
    inline bool Quaternion<FPType>::hasRotation(void) const {
        return (x == 0.0) && (y == 0.0) && (z == 0.0) && (w == 1.0);
    }

    template<typename FPType>
    inline Vector3<FPType> Quaternion<FPType>::rotate(const Vector3<FPType> v) {
        Quaternion ret = *this * Quaternion(v.x, v.y, v.z, 0.0) * Quaternion::conjugated(*this);
        return Vector3<FPType>(ret.x, ret.y, ret.z);
    }
    
    template<typename FPType>
    inline Quaternion<FPType> Quaternion<FPType>::makeRotation(const FPType rad, const FPType ax, const FPType ay, const FPType az) {
        Quaternion<FPType> q;
        FPType s = sin(rad * 0.5);
        FPType c = cos(rad * 0.5);
        
        q.x = ax * s;
        q.y = ay * s;
        q.z = az * s;
        q.w = c;
        
        return q;
    }
    
    template<typename FPType>
    void Quaternion<FPType>::sprint(char *buf, const Quaternion<FPType> q) {
        sprintf(buf, "quat(%.4lf,%.4lf,%.4lf,%.4lf)", (double)q.x, (double)q.y, (double)q.z, (double)q.w);
    }
    
    template<typename FPType>
    inline Quaternion<FPType> Quaternion<FPType>::normalized(const Quaternion<FPType> q) {
        Quaternion<FPType> ret = q;
        ret.normalize();
        return ret;
    }
    
    template<typename FPType>
    inline Quaternion<FPType> Quaternion<FPType>::inversed(const Quaternion<FPType> q) {
        Quaternion<FPType> ret = q;
        ret.inverse();
        return ret;
    }
    
    template<typename FPType>
    inline Quaternion<FPType> Quaternion<FPType>::conjugated(const Quaternion<FPType> q) {
        Quaternion<FPType> ret = q;
        ret.conjugate();
        return ret;
    }
    
    template<typename FPType>
    inline Quaternion<FPType> Quaternion<FPType>::lerp(const Quaternion<FPType> q0, const Quaternion<FPType> q1, const FPType t) {
        Quaternion<FPType> q;
        FPType t0 = 1.0 - t;
        q.x = q0.x * t0 + q1.x * t;
        q.y = q0.y * t0 + q1.y * t;
        q.z = q0.z * t0 + q1.z * t;
        q.w = q0.w * t0 + q1.w * t;
        return q;
    }
    
    template<typename FPType>
    inline Quaternion<FPType> Quaternion<FPType>::slerp(const Quaternion<FPType> q0, const Quaternion<FPType> q1, const FPType t) {
        Quaternion<FPType> q;
        FPType cosphi = std::max(0.0, std::min(1.0, q0.x * q1.x + q0.y * q1.y + q0.z * q1.z + q0.w * q1.w));
        FPType sinphi = 1.0 - cosphi * cosphi;
        if (sinphi < kEPS) {
            return q0;
        }
        sinphi = sqrt(sinphi);
        FPType phi = acos(cosphi);
        FPType t0 = sin(phi * (1.0 - t)) / sinphi;
        FPType t1 = sin(phi * t) / sinphi;
        q.x = q0.x * t0 + q1.x * t1;
        q.y = q0.y * t0 + q1.y * t1;
        q.z = q0.z * t0 + q1.z * t1;
        q.w = q0.w * t0 + q1.w * t1;
        return q;
    }
    
    template<typename FPType>
    inline Quaternion<FPType> Quaternion<FPType>::operator+(const Quaternion<FPType> &b) const {
        return Quaternion(x + b.x, y + b.y, z + b.z, w + b.w);
    }

    template<typename FPType>
    inline Quaternion<FPType> Quaternion<FPType>::operator-(const Quaternion<FPType> &b) const {
        return Quaternion(x - b.x, y - b.y, z - b.z, w - b.w);
    }

    template<typename FPType>
    inline Quaternion<FPType> Quaternion<FPType>::operator*(const Quaternion<FPType> &b) const {
        Quaternion q;
        q.x = w * b.x + x * b.w + y * b.z - z * b.y;
        q.y = w * b.y - x * b.z + y * b.w + z * b.x;
        q.z = w * b.z + x * b.y - y * b.x + z * b.w;
        q.w = w * b.w - x * b.x - y * b.y - z * b.z;
        return q;
    }

    template<typename FPType>
    inline Quaternion<FPType> Quaternion<FPType>::operator/(const Quaternion<FPType> &b) const {
        return *this * Quaternion<FPType>::inversed(b);
    }

    template<typename FPType>
    inline Quaternion<FPType> Quaternion<FPType>::operator*(const FPType s) const {
        return Quaternion(x * s, y * s, z * s, w * s);
    }

    template<typename FPType>
    inline Quaternion<FPType> Quaternion<FPType>::operator/(const FPType s) const {
        return Quaternion(x / s, y / s, z / s, w / s);
    }
    
    template<typename FPType>
    inline Quaternion<FPType> Quaternion<FPType>::operator+=(const Quaternion<FPType> &b) {
        x += b.x;
        y += b.y;
        z += b.z;
        w += b.w;
        return *this;
    }
    
    template<typename FPType>
    inline Quaternion<FPType> Quaternion<FPType>::operator-=(const Quaternion<FPType> &b) {
        x -= b.x;
        y -= b.y;
        z -= b.z;
        w -= b.w;
        return *this;
    }

    template<typename FPType>
    inline const Quaternion<FPType> operator*(const FPType s, const Quaternion<FPType> &q) {
        return q * s;
    }
    template<typename FPType>
    inline const Quaternion<FPType> operator/(const FPType s, const Quaternion<FPType> &q) {
        return q / s;
    }

}

#endif
