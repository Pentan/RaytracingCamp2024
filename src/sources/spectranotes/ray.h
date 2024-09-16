#ifndef PINKYPI_RAY_H
#define PINKYPI_RAY_H

#include <cmath>
#include "pptypes.h"

namespace PinkyPi {
    
    struct Ray {
        Vector3 origin;
        Vector3 direction;
        
        Ray() :
            origin(0.0, 0.0, 0.0),
            direction(0.0, 0.0, -1.0)
        {};
        
        Ray(const Vector3 &org, const Vector3 &dir):
            origin(org),
            direction(dir)
        {};
        
        Ray transformed(Matrix4 m) const {
            Ray ret = *this;
            ret.origin = Matrix4::transformV3(m, origin);
            ret.direction = Matrix4::mulV3(m, direction);
            ret.direction.normalize();
            return ret;
        }
        
        Vector3 pointAt(PPFloat t) const {
            return direction * t + origin;
        }
    };
}
#endif
