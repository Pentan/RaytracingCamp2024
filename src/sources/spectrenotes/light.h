#ifndef PINKYPI_LIGHT_H
#define PINKYPI_LIGHT_H

#include <string>
#include <vector>
#include <memory>
#include "pptypes.h"

namespace PinkyPi {
    
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
            PPFloat lightPdf;
        };
        
    public:
        Light();
        ~Light();

        Color evaluate(const Node* node, const SurfaceInfo& surf, PPTimeType timerate, EvalLog* log) const;
        
        std::string name;
        Color color;
        PPFloat intensity;
        LightType lightType;
        
        struct Spot {
            PPFloat innerConeAngle;
            PPFloat outerConeAngle;
            
            Spot(PPFloat inner, PPFloat outer):
                innerConeAngle(inner),
                outerConeAngle(outer)
            {}
        } spot;
    };
}

#endif
