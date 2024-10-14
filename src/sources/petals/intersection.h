//
//  intersection.h
//  Petals
//
//  Created by SatoruNAKAJIMA on 2022/08/14.
//

#ifndef PETALS_INTERSECTION_H
#define PETALS_INTERSECTION_H

#include "types.h"

namespace Petals {
    //
    struct MeshIntersection {
        int meshId;
        int clusterId;
        int triangleId;
        RTFloat vcb;
        RTFloat vcc;
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
#endif /* PETALS_INTERSECTION_H */
