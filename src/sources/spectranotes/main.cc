#include <cstdio>
#include <iostream>

#include <pinkycore/assetlibrary.h>
#include <pinkycore/scene.h>
#include <pinkycore/bvh.h>
#include <pinkycore/renderer.h>
#include <pinkycore/config.h>
#include <pinkycore/postprocessor.h>

#include "sceneloader.h"

int main(int argc, char* argv[])
{
    std::cout << "Hellooooo Pinky Pi!\n" << std::endl;
    
    std::string configPath = "data/config.json";
    PinkyPi::Config config;
    if(!config.load(configPath)) {
        std::cerr << "config load failed. use default settings." << std::endl;
    }
    if(argc > 1) {
        config.parseOptions(argc, argv);
    }
    
    config.print();
    
    // check output dir?
    
    PinkyPi::AssetLibrary *assetlib = nullptr;
    PinkyPi::Scene *scene = nullptr;
    
    if(config.inputFile.length() > 0) {
        assetlib = PinkyPi::SceneLoader::load(config.inputFile);
        scene = assetlib->getDefaultScene();
    } else {
//        scene = PinkyPi::Scene::buildDefaultScene();
    }
    
    if(scene == nullptr) {
        std::cerr << "scene is nullptr" << std::endl;
        return 0;
    }
    
    scene->preprocess(&config);
    
    PinkyPi::Renderer renderer(config, scene);
    renderer.render();
    
    delete scene;
    
    return 0;
}
