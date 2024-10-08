#ifndef SPECTRENOTES_CAMERA_H
#define SPECTRENOTES_CAMERA_H

#include <string>
#include <vector>
#include <memory>
#include "types.h"
#include "ray.h"
#include "random.h"

namespace Spectrenotes {
    
    class Camera {
    public:
        enum CameraType {
            kPerspectiveCamera,
            kOrthographicsCamera,
            kFocusPlanePerspectiveCamera,
        };
        
    public:
        Camera();
        ~Camera();
        
        void initWithType(CameraType t);
        
        // tx and ty range is (-1,1)
        Ray getRay(RTFloat tx, RTFloat ty, Random* rng);

        static Ray getThinLensRay(Camera* cam, RTFloat tx, RTFloat ty, Random* rng);
        static Ray getPerspectiveRay(Camera* cam, RTFloat tx, RTFloat ty, Random* rng);
        static Ray getOrthoRay(Camera* cam, RTFloat tx, RTFloat ty, Random* rng);
        static Ray getThinLensRayFromFocusPlane(Camera* cam, RTFloat tx, RTFloat ty, Random* rng);

        Ray(*getRayFunc)(Camera*, RTFloat, RTFloat, Random*);

        //
        std::string name;
        CameraType type;
        
        union {
            struct {
                RTFloat aspect;
                RTFloat yfov;
                RTFloat zfar;
                RTFloat znear;
            } perspective;

            struct {
                RTFloat xmag;
                RTFloat ymag;
                RTFloat zfar;
                RTFloat znear;
            } orthographics;
        };

        RTFloat sensorWidth;    // [mm]
        RTFloat sensorHeight;   // [mm]
        RTFloat focalLength;    // [mm]
        RTFloat fNumber;        // [focal length / diameter of entrance pupil]
        RTFloat focusDistance;  // [m]
        RTFloat focusPlaneWidth;  // [m]
        RTFloat focusPlaneHeight; // [m]
    };
}

#endif
