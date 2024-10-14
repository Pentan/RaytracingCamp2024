#ifndef PETALS_LIGHT_H
#define PETALS_LIGHT_H

#include <string>
#include <vector>
#include <memory>
#include "types.h"

namespace Petals {
    
    class Node;

    class Light {
    public:
        enum LightType {
            kPointLight,
            kSpotLight,
            kDirectionalLight,
            kMeshLight
        };

        struct EvalLog {
            Vector3 position;
            Vector3 direction;
            RTFloat lightPdf;
        };
        
    public:
        Light();
        ~Light();

        Color evaluate(const Node* node, const SurfaceInfo& surf, RTTimeType timerate, EvalLog* log) const;
        
        std::string name;
        Color color;
        RTFloat intensity;
        LightType lightType;
        
        struct Spot {
            RTFloat innerConeAngle;
            RTFloat outerConeAngle;
            
            Spot(RTFloat inner, RTFloat outer):
                innerConeAngle(inner),
                outerConeAngle(outer)
            {}
        } spot;
    };
}

#endif
