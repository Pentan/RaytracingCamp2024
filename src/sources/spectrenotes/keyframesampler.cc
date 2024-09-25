#include "keyframesampler.h"

using namespace Spectrenotes;

//int KeyframeSampler::nearestKeyframeIndex(RTTimeType time) const {
//    const int keyLastIndex = static_cast<int>(timeStamps.size() - 1);
//    int keyindex;
//    
//    if(time < timeStamps[0]) {
//        keyindex = 0;
//    } else if(time > timeStamps[keyLastIndex]) {
//        keyindex = keyLastIndex;
//    } else {
//        keyindex = static_cast<int>((time - timeStamps[0]) / (timeStamps[keyLastIndex] - timeStamps[0]));
//        for(; keyindex <= keyLastIndex; keyindex++) {
//            if(time < timeStamps[keyindex]) {
//                break;
//            }
//        }
//        for(; keyindex >= 0; keyindex--) {
//            if(time >= timeStamps[keyindex]) {
//                break;
//            }
//        }
//    }
//    return keyindex;
//}

KeyframeSampler::KeyWeights KeyframeSampler::calclateKeyWeights(RTTimeType time) const {
    KeyWeights ret;
    const int keycount = static_cast<int>(timeStamps.size());
    const int keyLastIndex = keycount - 1;
    int nearestIndex;
    
    if(time < timeStamps[0]) {
        nearestIndex = 0;
        time = timeStamps[0];
    } else if(time >= timeStamps[keyLastIndex]) {
        nearestIndex = keyLastIndex;
        time = timeStamps[keyLastIndex];
    } else {
        nearestIndex = static_cast<int>((time - timeStamps[0]) / (timeStamps[keyLastIndex] - timeStamps[0])) * keycount;
        for(; nearestIndex <= keyLastIndex; nearestIndex++) {
            if(time < timeStamps[nearestIndex]) {
                break;
            }
        }
        for(; nearestIndex >= 0; nearestIndex--) {
            if(time >= timeStamps[nearestIndex]) {
                break;
            }
        }
    }
    
    switch (interpolation) {
        case kStep:
            ret.keyindex[0] = nearestIndex;
            ret.keyindex[1] = nearestIndex;
            ret.weights[0] = 1.0;
            ret.weights[1] = 0.0;
            break;
        case kCubicSpline:
            // FIXME
            // break; // fallback to LINEAR
        default:
        case kLinear:
        {
            ret.keyindex[0] = nearestIndex;
            ret.keyindex[1] = std::min(nearestIndex + 1, keyLastIndex);
            if(ret.keyindex[0] == ret.keyindex[1]) {
                ret.weights[0] = 1.0;
                ret.weights[1] = 0.0;
            } else {
                RTTimeType ts0 = timeStamps[ret.keyindex[0]];
                RTTimeType ts1 = timeStamps[ret.keyindex[1]];
                ret.weights[1] = (time - ts0) / (ts1 - ts0);
                ret.weights[0] = 1.0 - ret.weights[1];
            }
        }
            break;
    }
    
    return ret;
}

void KeyframeSampler::sample(RTTimeType time, std::vector<RTFloat>& outbuf) {
    KeyWeights kw = calclateKeyWeights(time);
    RTFloat* data0 = sampleBuffer.data() + kw.keyindex[0] * sampleComponents;
    RTFloat* data1 = sampleBuffer.data() + kw.keyindex[1] * sampleComponents;

    outbuf.resize(sampleComponents);
    for (size_t i = 0; i < sampleComponents; i++) {
        switch (interpolation) {
        case kStep:
            outbuf[i] = data0[i];
            break;
        case kCubicSpline:
            // FIXME
            // break; // fallback to LINEAR
        default:
        case kLinear:
            outbuf[i] = data0[0] * kw.weights[0] + data1[0] * kw.weights[1];
            break;
        }
    }
}

Vector3 KeyframeSampler::sampleVector3(RTTimeType time) {
    KeyWeights kw = calclateKeyWeights(time);
    RTFloat* data0 = sampleBuffer.data() + kw.keyindex[0] * sampleComponents;
    RTFloat* data1 = sampleBuffer.data() + kw.keyindex[1] * sampleComponents;
    
    Vector3 retv;
    switch (interpolation) {
        case kStep:
            retv.set(data0[0], data0[1], data0[2]);
            break;
        case kCubicSpline:
            // FIXME
            // break; // fallback to LINEAR
        default:
        case kLinear:
            retv.set(
                data0[0] * kw.weights[0] + data1[0] * kw.weights[1],
                data0[1] * kw.weights[0] + data1[1] * kw.weights[1],
                data0[2] * kw.weights[0] + data1[2] * kw.weights[1]);
            break;
    }
    return retv;
}

Vector4 KeyframeSampler::sampleVector4(RTTimeType time) {
    KeyWeights kw = calclateKeyWeights(time);
    RTFloat* data0 = sampleBuffer.data() + kw.keyindex[0] * sampleComponents;
    RTFloat* data1 = sampleBuffer.data() + kw.keyindex[1] * sampleComponents;
    
    Vector4 retv;
    switch (interpolation) {
        case kStep:
            retv.set(data0[0], data0[1], data0[2], data0[3]);
            break;
        case kCubicSpline:
            // FIXME
            // break; // fallback to LINEAR
        default:
        case kLinear:
            retv.set(
                data0[0] * kw.weights[0] + data1[0] * kw.weights[1],
                data0[1] * kw.weights[0] + data1[1] * kw.weights[1],
                data0[2] * kw.weights[0] + data1[2] * kw.weights[1],
                data0[3] * kw.weights[0] + data1[3] * kw.weights[1]);
            break;
    }
    return retv;
}

Quaterion KeyframeSampler::sampleQuaternion(RTTimeType time) {
    KeyWeights kw = calclateKeyWeights(time);
    RTFloat* data0 = sampleBuffer.data() + kw.keyindex[0] * sampleComponents;
    RTFloat* data1 = sampleBuffer.data() + kw.keyindex[1] * sampleComponents;
    
    Quaterion retq;
    switch (interpolation) {
        case kStep:
            retq.set(data0[0], data0[1], data0[2], data0[3]);
            break;
        case kCubicSpline:
            // FIXME
            // break; // fallback to LINEAR
        default:
        case kLinear:
        {
            Quaterion q0(data0[0], data0[1], data0[2], data0[3]);
            Quaterion q1(data1[0], data1[1], data1[2], data1[3]);
            retq = Quaterion::slerp(q0, q1, kw.weights[1]);
        }
            break;
    }
    
    return retq;
}
