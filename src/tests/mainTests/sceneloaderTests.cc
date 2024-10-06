#include <string>
#include <sstream>

#include <doctest.h>
#include "../testsupport.h"

#include <spectrenotes/types.h>
#include <spectrenotes/assetlibrary.h>
#include <spectrenotes/scene.h>
#include <spectrenotes/bvh.h>
#include <spectrenotes/sceneloader.h>

using namespace Spectrenotes;

namespace {
    std::string KhronosGlTFSampleModelPath(std::string name) {
        std::stringstream ss;
        ss <<  SPCTRNTS_TEST_DATA_DIR;
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
    ss << SPCTRNTS_TEST_DATA_DIR << "/" << "test_scene01.json";
    std::string path = ss.str();

    AssetLibrary* assetlib = SceneLoader::loadAnimStand(path);
    REQUIRE(assetlib != nullptr);
}
