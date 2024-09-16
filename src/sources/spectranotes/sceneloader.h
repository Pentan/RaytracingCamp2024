#ifndef PINKYPI_SCENE_LOADER_H
#define PINKYPI_SCENE_LOADER_H

namespace PinkyPi {
    
    class AssetLibrary;
    
    class SceneLoader {
    public:
        // this is thread unsafe
        static AssetLibrary* load(std::string filepath);
    };
}

#endif
