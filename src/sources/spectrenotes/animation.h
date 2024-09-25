#ifndef PINKYPI_ANIMATION_H
#define PINKYPI_ANIMATION_H

#include <string>
#include <vector>
#include <memory>
#include "pptypes.h"

namespace PinkyPi {
    class KeyframeSampler;
    class Node;

    class Animation {
        public:
        enum TargetProperty {
            kTranslation,
            kRotation,
            kScale,
            kMorphWeights
        };

        struct AnimationTarget {
            KeyframeSampler* sampler;
            Node* node;
            TargetProperty targetProp;
        };

        std::string name;
        std::vector<std::shared_ptr<KeyframeSampler> > samplers;
        std::vector<AnimationTarget> targets;
    };
}

#endif
