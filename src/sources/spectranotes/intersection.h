//
//  intersection.h
//  PinkyCore
//
//  Created by SatoruNAKAJIMA on 2022/08/14.
//

#ifndef PINKYPI_INTERSECTION_H
#define PINKYPI_INTERSECTION_H

#include "pptypes.h"

namespace PinkyPi {
    //
    struct MeshIntersection {
        int meshId;
        int clusterId;
        int triangleId;
        PPFloat vcb;
        PPFloat vcc;
    };
    
    /////
    struct SceneIntersection {
        int tracableId;
        MeshIntersection meshIntersect;
    };

    /////
    struct IntersectionDetail {
        Vector3 geometryNormal;

        Vector3 barycentricCoord;
        Vector3 shadingNormal;
        Vector4 shadingTangent;
        Vector3 texcoord0;

        Attributes vertexAttributes[3];
        int uvCount;
        int colorCount;
        
        int materialId;
    };
}
#endif /* PINKYPI_INTERSECTION_H */
