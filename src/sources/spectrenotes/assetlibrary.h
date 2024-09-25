#ifndef PINKYPI_ASSETLIBRARY_H
#define PINKYPI_ASSETLIBRARY_H

#include <string>
#include <vector>
#include <memory>

namespace PinkyPi {
    
    class Mesh;
    class Material;
    class Texture;
    class Light;
    class Node;
    class Camera;
    class Scene;
    class Animation;
    class Skin;
    
    class AssetLibrary {
    public:
        AssetLibrary();
        ~AssetLibrary();
        
        Scene* getDefaultScene() const { return (scenes.size() > defaultSceneId) ? scenes[defaultSceneId].get() : nullptr; }
        
        Texture* getTexture(int i) const    { return textures[i].get(); }
        Material* getMaterial(int i) const  { return materials[i].get(); }
        // Material* getMaterial(int meshId, int clusterId) const;
        Mesh* getMesh(int i) const          { return meshes[i].get(); }
        Light* getLight(int i) const        { return lights[i].get(); }
        Node* getNode(int i) const          { return nodes[i].get(); }
        Camera* getCamera(int i) const      { return cameras[i].get(); }
        Scene* getScene(int i) const        { return scenes[i].get(); }
        
        std::vector<std::shared_ptr<Material> > materials;
        std::vector<std::shared_ptr<Texture> > textures;
        std::vector<std::shared_ptr<Mesh> > meshes;
        std::vector<std::shared_ptr<Light> > lights;
        std::vector<std::shared_ptr<Camera> > cameras;
        std::vector<std::shared_ptr<Node> > nodes;
        std::vector<std::shared_ptr<Skin> > skins;
        std::vector<std::shared_ptr<Animation> > animations;
        std::vector<std::shared_ptr<Scene> > scenes;
        
        std::shared_ptr<Texture> backgroundTex;
        
        int defaultSceneId;
    };
}

#endif
