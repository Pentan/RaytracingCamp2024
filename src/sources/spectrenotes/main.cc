#include <cstdio>
#include <iostream>

#include <spectrenotes/assetlibrary.h>
#include <spectrenotes/scene.h>
#include <spectrenotes/bvh.h>
#include <spectrenotes/renderer.h>
#include <spectrenotes/config.h>
#include <spectrenotes/postprocessor.h>
#include <spectrenotes/animstand.h>

#include "sceneloader.h"

int main(int argc, char* argv[])
{
    std::cout << "===== Petals =====" << std::endl;
    
    std::string configPath = "etc/config.json";
    Spectrenotes::Config config;
    if(!config.load(configPath)) {
        std::cerr << "config load failed. use default settings." << std::endl;
    }
    if(argc > 1) {
        config.parseOptions(argc, argv);
    }

#if 0
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
#else
    Spectrenotes::AnimationStand* animstand = nullptr;

    if (config.inputFile.length() > 0) {
        std::cout << "input file: " << config.inputFile << std::endl;
        animstand = Spectrenotes::SceneLoader::loadAnimStand(config.inputFile);
        std::cout << " loaded!" << std::endl;
    }
    else {
        std::cout << "no input file. use -i option or edit config file." << std::endl;
    }

    if (animstand) {
        // config
        animstand->maxThreads = config.maxThreads;
        animstand->limitSec = config.limitSec;

        std::cout << "start rendering" << std::endl;
        animstand->render();
        std::cout << "done" << std::endl;
        delete animstand;
    }

#endif
    return 0;
}
