#ifndef PINKYPI_CAMERA_H
#define PINKYPI_CAMERA_H

#include <string>
#include <vector>
#include <memory>
#include "pptypes.h"
#include "ray.h"
#include "random.h"

namespace PinkyPi {
    
    class Camera {
    public:
        enum CameraType {
            kPerspectiveCamera,
            kOrthographicsCamera
        };
        
    public:
        Camera();
        ~Camera();
        
        void initWithType(CameraType t);
        
        // tx and ty range is (-1,1)
        Ray getRay(PPFloat tx, PPFloat ty, Random* rng);

        static Ray getThinLensRay(Camera* cam, PPFloat tx, PPFloat ty, Random* rng);
        static Ray getPerspectiveRay(Camera* cam, PPFloat tx, PPFloat ty, Random* rng);
        static Ray getOrthoRay(Camera* cam, PPFloat tx, PPFloat ty, Random* rng);

        Ray(*getRayFunc)(Camera*, PPFloat, PPFloat, Random*);

        //
        std::string name;
        CameraType type;
        
        union {
            struct {
                PPFloat aspect;
                PPFloat yfov;
                PPFloat zfar;
                PPFloat znear;
            } perspective;

            struct {
                PPFloat xmag;
                PPFloat ymag;
                PPFloat zfar;
                PPFloat znear;
            } orthographics;
        };

        // PPFloat focalLength;    // [mm]
        PPFloat fNumber;        // [focal length / diameter of entrance pupil]
        PPFloat focusDistance;  // [m]
    };
}

#endif
