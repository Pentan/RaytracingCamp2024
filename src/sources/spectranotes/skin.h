#ifndef PINKYPI_SKIN_H
#define PINKYPI_SKIN_H

#include <string>
#include <vector>
#include "pptypes.h"

namespace PinkyPi {
    class Node;
    
    class Skin {
    public:
        Skin(){}
        
        std::string name;
        Node* skeltonRoot;
        std::vector<Node*> jointNodes;
        std::vector<Matrix4> inverseBindMatrices;
        
        // TODO skined mesh cache
    };
}

#endif
