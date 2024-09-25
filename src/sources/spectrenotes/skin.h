#ifndef SPECTRENOTES_SKIN_H
#define SPECTRENOTES_SKIN_H

#include <string>
#include <vector>
#include "types.h"

namespace Spectrenotes {
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
