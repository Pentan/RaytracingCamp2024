#ifndef PINKYPI_KEYFRAMESAMPLER_H
#define PINKYPI_KEYFRAMESAMPLER_H

#include <string>
#include <vector>
#include "pptypes.h"

namespace PinkyPi {
    
    class KeyframeSampler {
    public:
        enum InterpolationType {
            kLinear,
            kStep,
            kCubicSpline
        };
        
        struct KeyWeights {
            int keyindex[2];
            PPFloat weights[2];
        };
        
        KeyframeSampler()
            : interpolation(kLinear)
        {}
        
//        int nearestKeyframeIndex(PPTimeType time) const;
        KeyWeights calclateKeyWeights(PPTimeType time) const;
        
        void sample(PPTimeType time, std::vector<PPFloat>& outbuf);
        Vector3 sampleVector3(PPTimeType time);
        Vector4 sampleVector4(PPTimeType time);
        Quaterion sampleQuaternion(PPTimeType time);
        
        InterpolationType interpolation;
        std::vector<PPTimeType> timeStamps;
        std::vector<PPFloat> sampleBuffer;
        size_t sampleComponents;
    };
}

#endif
