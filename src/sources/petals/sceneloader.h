#ifndef PETALS_SCENE_LOADER_H
#define PETALS_SCENE_LOADER_H

#include <string>

namespace Petals {
    
    class AssetLibrary;
    class AnimationStand;
    
    class SceneLoader {
    public:
        // these methods are thread unsafe
        static AssetLibrary* loadGLTF(std::string filepath);
        static AnimationStand* loadAnimStand(std::string filepath);
    };
}

#endif
