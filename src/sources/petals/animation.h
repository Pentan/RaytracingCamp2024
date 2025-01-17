#ifndef PETALS_ANIMATION_H
#define PETALS_ANIMATION_H

#include <string>
#include <vector>
#include <memory>
#include "types.h"

namespace Petals {
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
