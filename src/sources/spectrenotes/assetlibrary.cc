
#include "types.h"
#include "assetlibrary.h"
#include "mesh.h"
#include "bvh.h"
#include "material.h"
#include "texture.h"
#include "light.h"
#include "node.h"
#include "camera.h"
#include "scene.h"

using namespace Spectrenotes;

AssetLibrary::AssetLibrary():
    defaultSceneId(-1)
{}

AssetLibrary::~AssetLibrary() {
    
}

//Material* AssetLibrary::getMaterial(int meshId, int clusterId) const {
//    return meshes[meshId]->clusters[clusterId]->material;
//}
