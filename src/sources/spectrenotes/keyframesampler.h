#ifndef SPECTRENOTES_KEYFRAMESAMPLER_H
#define SPECTRENOTES_KEYFRAMESAMPLER_H

#include <string>
#include <vector>
#include "types.h"

namespace Spectrenotes {
    
    class KeyframeSampler {
    public:
        enum InterpolationType {
            kLinear,
            kStep,
            kCubicSpline
        };
        
        struct KeyWeights {
            int keyindex[2];
            RTFloat weights[2];
        };
        
        KeyframeSampler()
            : interpolation(kLinear)
        {}
        
//        int nearestKeyframeIndex(RTTimeType time) const;
        KeyWeights calclateKeyWeights(RTTimeType time) const;
        
        void sample(RTTimeType time, std::vector<RTFloat>& outbuf);
        Vector3 sampleVector3(RTTimeType time);
        Vector4 sampleVector4(RTTimeType time);
        Quaterion sampleQuaternion(RTTimeType time);
        
        InterpolationType interpolation;
        std::vector<RTTimeType> timeStamps;
        std::vector<RTFloat> sampleBuffer;
        size_t sampleComponents;
    };
}

#endif
