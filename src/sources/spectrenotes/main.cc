#include <cstdio>
#include <iostream>

#include <spectrenotes/assetlibrary.h>
#include <spectrenotes/scene.h>
#include <spectrenotes/bvh.h>
#include <spectrenotes/renderer.h>
#include <spectrenotes/config.h>
#include <spectrenotes/postprocessor.h>

#include "sceneloader.h"

int main(int argc, char* argv[])
{
    std::cout << "===== Spectrenotes =====\n" << std::endl;
    
    std::string configPath = "data/config.json";
    Spectrenotes::Config config;
    if(!config.load(configPath)) {
        std::cerr << "config load failed. use default settings." << std::endl;
    }
    if(argc > 1) {
        config.parseOptions(argc, argv);
    }
    
    config.print();
    
    // check output dir?
    
    Spectrenotes::AssetLibrary *assetlib = nullptr;
    Spectrenotes::Scene *scene = nullptr;
    
    if(config.inputFile.length() > 0) {
        assetlib = Spectrenotes::SceneLoader::loadGLTF(config.inputFile);
        scene = assetlib->getDefaultScene();
    } else {
//        scene = Spectrenotes::Scene::buildDefaultScene();
    }
    
    if(scene == nullptr) {
        std::cerr << "scene is nullptr" << std::endl;
        return 0;
    }
    
    scene->preprocess(&config);
    
    Spectrenotes::Renderer renderer(config, scene);
    renderer.render();
    
    delete scene;
    
    return 0;
}
