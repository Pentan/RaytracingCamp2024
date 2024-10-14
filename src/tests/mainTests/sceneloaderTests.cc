#include <string>
#include <sstream>

#include <doctest.h>
#include "../testsupport.h"

#include <petals/types.h>
#include <petals/assetlibrary.h>
#include <petals/scene.h>
#include <petals/bvh.h>
#include <petals/sceneloader.h>
#include <petals/animstand.h>

using namespace Petals;

namespace {
    std::string KhronosGlTFSampleModelPath(std::string name) {
        std::stringstream ss;
        ss <<  PETALS_TEST_DATA_DIR;
        ss << "/glTF-Sample-Assets/Models/";
        ss << name << "/glTF/";
        ss << name << ".gltf";
        return ss.str();
    }
}

TEST_CASE("Scene loader test [SceneLoader] [glTF]") {
    std::string gltfpath = KhronosGlTFSampleModelPath("Box");
    AssetLibrary *assetlib = SceneLoader::loadGLTF(gltfpath);
    REQUIRE(assetlib != nullptr);
    Scene *scene = assetlib->getDefaultScene();
    REQUIRE(scene != nullptr);
}

TEST_CASE("Skin test [SceneLoader] [glTF]") {
    std::string gltfpath = KhronosGlTFSampleModelPath("SimpleSkin");
    AssetLibrary* assetlib = SceneLoader::loadGLTF(gltfpath);
    REQUIRE(assetlib != nullptr);
    Scene* scene = assetlib->getDefaultScene();
    REQUIRE(scene != nullptr);
}

TEST_CASE("Morph test [SceneLoader] [glTF]") {
    std::string gltfpath = KhronosGlTFSampleModelPath("AnimatedMorphCube");
    AssetLibrary* assetlib = SceneLoader::loadGLTF(gltfpath);
    REQUIRE(assetlib != nullptr);
    Scene* scene = assetlib->getDefaultScene();
    REQUIRE(scene != nullptr);
}

TEST_CASE("Animated model test 01 [SceneLoader] [glTF]") {
    std::string gltfpath = KhronosGlTFSampleModelPath("Fox");
    AssetLibrary* assetlib = SceneLoader::loadGLTF(gltfpath);
    REQUIRE(assetlib != nullptr);
    Scene* scene = assetlib->getDefaultScene();
    REQUIRE(scene != nullptr);
}

TEST_CASE("Cell stage scene test 01 [SceneLoader] [AnimStand]") {
    std::stringstream ss;
    ss << PETALS_TEST_DATA_DIR << "/" << "test_scene01.json";
    std::string path = ss.str();

    AnimationStand* animstand = SceneLoader::loadAnimStand(path);
    REQUIRE(animstand != nullptr);

    //animstand->outconf.directory = PETALS_TEST_OUTPUT_DIR;
    //animstand->render();

    // delete animstand;
}
