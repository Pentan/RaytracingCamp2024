#ifndef SPECTRENOTES_SCENE_LOADER_H
#define SPECTRENOTES_SCENE_LOADER_H

namespace Spectrenotes {
    
    class AssetLibrary;
    
    class SceneLoader {
    public:
        // this is thread unsafe
        static AssetLibrary* load(std::string filepath);
    };
}

#endif
