//
//  light.cpp
//  Spectrenotes
//
//  Created by SatoruNAKAJIMA on 2019/08/16.
//

#include "light.h"
#include "types.h"
#include "node.h"

using namespace Spectrenotes;

Light::Light():
    color(0.0, 0.0, 0.0),
    intensity(1.0),
    lightType(kPointLight),
    spot(0.0, kPI * 0.25)
{
}

Light::~Light() {
    
}

Color Light::evaluate(const Node* node, const SurfaceInfo& surf, RTTimeType timerate, EvalLog* log) const {
    Color ret;

    switch (lightType)
    {
        case kPointLight:
        {
            Vector3 lp = Matrix4::transformV3(node->computeGlobalMatrix(timerate), Vector3(0.0, 0.0, 0.0));
            Vector3 lv = lp - surf.position;
            RTFloat ll = lv.length();
            lv = lv / ll;
            ret = color * intensity / (ll * ll);

            log->lightPdf = 1.0;
            log->position = lp;
            log->direction = lv;
        }
            break;
        case kDirectionalLight:
            break;
        case kSpotLight:
            break;
        case kMeshLight:
            break;
        default:
            break;
    }

    return ret;
}
