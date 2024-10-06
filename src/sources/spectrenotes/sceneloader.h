#ifndef SPECTRENOTES_SCENE_LOADER_H
#define SPECTRENOTES_SCENE_LOADER_H

#include <string>

namespace Spectrenotes {
    
    class AssetLibrary;
    
    class SceneLoader {
    public:
        // these methods are thread unsafe
        static AssetLibrary* loadGLTF(std::string filepath);
        static AssetLibrary* loadAnimStand(std::string filepath);
    };
}

#endif
