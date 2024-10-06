#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <vector>
#include <memory>
#include <map>

#include <nlohmann/json.hpp>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

#define TINYGLTF_NO_INCLUDE_JSON
#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE
#include <tinygltf/tiny_gltf.h>

#include <spectrenotes/assetlibrary.h>
#include <spectrenotes/scene.h>
#include <spectrenotes/material.h>
#include <spectrenotes/texture.h>
#include <spectrenotes/mesh.h>
#include <spectrenotes/light.h>
#include <spectrenotes/node.h>
#include <spectrenotes/camera.h>
#include <spectrenotes/animation.h>
#include <spectrenotes/keyframesampler.h>
#include <spectrenotes/skin.h>
#include <spectrenotes/bvh.h>

#include "sceneloader.h"

using namespace Spectrenotes;

namespace {
    // Constants
    std::string kExtraPrefix = "Spectrenotes-Extra-";

    // Temporal vars
    struct SkinedMeshNode {
        int skinIndex;
        Node* node;
    };
    std::vector<SkinedMeshNode>* skinedMeshNodes;
    std::map<int, std::string>* bgImagePaths;
    
    // Utility Classes
    class BufferAccessor {
    private:
        const unsigned char* bufferPtr;
        size_t byteStride;
        size_t componentSize;
        int componentsInStruct;
        int structCount;
        float componentNormalizer;
        
        int curStruct;
        int curComponent;
        
        float (BufferAccessor::*getCompFloat)(int, int) const;
        int (BufferAccessor::*getCompInt)(int, int) const;
        
        template<typename R, typename T> R getT(int structIndex, int componentIndex) const {
            T *val = (T*)(bufferPtr + byteStride * structIndex + componentIndex * componentSize);
            return R(*val);
        }
        
    public:
        BufferAccessor():
            bufferPtr(nullptr),
            byteStride(0),
            componentSize(0),
            componentsInStruct(1),
            structCount(0),
            componentNormalizer(1.0)
        {
            rewind();
        }
        
        BufferAccessor(const tinygltf::Accessor& accessor, const tinygltf::Model& model) {
            init(accessor, model);
        }
        
        void init(const tinygltf::Accessor& accessor, const tinygltf::Model& model) {
            auto& bufferview = model.bufferViews[accessor.bufferView];
            auto& buffer = model.buffers[bufferview.buffer];
            
            size_t offset = bufferview.byteOffset + accessor.byteOffset;
            bufferPtr = buffer.data.data() + offset;
            byteStride = bufferview.byteStride;
            structCount = static_cast<int>(accessor.count);
            
            switch (accessor.type) {
                case TINYGLTF_TYPE_SCALAR:
                    componentsInStruct = 1;
                    break;
                case TINYGLTF_TYPE_VEC2:
                    componentsInStruct = 2;
                    break;
                case TINYGLTF_TYPE_VEC3:
                    componentsInStruct = 3;
                    break;
                case TINYGLTF_TYPE_VEC4:
                    componentsInStruct = 4;
                    break;
                case TINYGLTF_TYPE_MAT2:
                    componentsInStruct = 4;
                    break;
                case TINYGLTF_TYPE_MAT3:
                    componentsInStruct = 9;
                    break;
                case TINYGLTF_TYPE_MAT4:
                    componentsInStruct = 16;
                    break;
                default:
                    // unknown type
                    std::cerr << "Unknown component type accessor:" << accessor.componentType << std::endl;
                    componentsInStruct = 1;
                    break;
            }
            
            switch (accessor.componentType) {
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                    componentSize = 1;
                    componentNormalizer = 1.0f / float(0xff);
                    getCompFloat = &BufferAccessor::getT<float, unsigned char>;
                    getCompInt = &BufferAccessor::getT<int, unsigned char>;
                    break;
                    
                case TINYGLTF_COMPONENT_TYPE_BYTE:
                    componentSize = 1;
                    componentNormalizer = 1.0f / float(0x7f);
                    getCompFloat = &BufferAccessor::getT<float, char>;
                    getCompInt = &BufferAccessor::getT<int, char>;
                    break;
                    
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                    componentSize = 2;
                    componentNormalizer = 1.0f / float(0xffff);
                    getCompFloat = &BufferAccessor::getT<float, unsigned short>;
                    getCompInt = &BufferAccessor::getT<int, unsigned short>;
                    break;
                    
                case TINYGLTF_COMPONENT_TYPE_SHORT:
                    componentSize = 2;
                    componentNormalizer = 1.0f / float(0x7fff);
                    getCompFloat = &BufferAccessor::getT<float, short>;
                    getCompInt = &BufferAccessor::getT<int, short>;
                    break;
                    
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                    componentSize = 4;
                    componentNormalizer = 1.0f / float(0x7fffffff);
                    getCompFloat = &BufferAccessor::getT<float, long>;
                    getCompInt = &BufferAccessor::getT<int, long>;
                    break;
                    
                default:
                case TINYGLTF_COMPONENT_TYPE_FLOAT:
                    componentSize = 4;
                    componentNormalizer = 1.0f;
                    getCompFloat = &BufferAccessor::getT<float, float>;
                    getCompInt = &BufferAccessor::getT<int, float>;
                    break;
            }
            
            if(byteStride == 0) {
                byteStride = componentSize * componentsInStruct;
            }
            rewind();
        }

        template<typename T>
        void initWithData(const void* data, int dataCount, int cmpsInStrust, int stride=0, unsigned int fltdiv=1) {
            bufferPtr = static_cast<const unsigned char*>(data);
            componentSize = sizeof(T);
            componentsInStruct = cmpsInStrust;
            structCount = dataCount;
            componentNormalizer = 1.0f / static_cast<float>(fltdiv);
            byteStride = (stride > 0) ? stride : componentSize * componentsInStruct;

            getCompFloat = &BufferAccessor::getT<float, T>;
            getCompInt = &BufferAccessor::getT<int, T>;

            rewind();
        }
        
        float getFloat(int totalComponentIndex) const {
            if(bufferPtr == nullptr) return 0;
            int structIndex = totalComponentIndex / componentsInStruct;
            int compIndex = totalComponentIndex - componentsInStruct * structIndex;
            return (this->*getCompFloat)(structIndex, compIndex) * componentNormalizer;
        }
        
        float getFloat(int structIndex, int componentIndex) const {
            if(bufferPtr == nullptr) return 0;
            return (this->*getCompFloat)(structIndex, componentIndex) * componentNormalizer;
        }
        
        int getInt(int structIndex, int componentIndex) const {
            if(bufferPtr == nullptr) return 0;
            return (this->*getCompInt)(structIndex, componentIndex);
        }
        
        int getStructCount() const {
            return structCount;
        }
        
        int getComponentCountInStruct() const {
            return componentsInStruct;
        }
        
        int getTotalComponentCount() const {
            return structCount * componentsInStruct;
        }
        
        void rewind() {
            curStruct = 0;
            curComponent = 0;
        }
        
        float readFloat() {
            float c = getFloat(curStruct, curComponent);
            updateReadPosition();
            return c;
        }
        
        int readInt() {
            int c = getInt(curStruct, curComponent);
            updateReadPosition();
            return c;
        }
        
    private:
        void updateReadPosition() {
            curComponent += 1;
            if(curComponent >= componentsInStruct) {
                curStruct += 1;
                curComponent = 0;
            }
        }
    };
    
    // Utility functions
    tinygltf::Value FindSpectrenotesExtra(const tinygltf::Value& val, const std::string& targetKey) {
        if(val.IsArray()) {
            size_t n = val.ArrayLen();
            for(size_t i = 0; i < n; i++) {
                auto v = FindSpectrenotesExtra(val.Get(static_cast<int>(i)), targetKey);
                if(v.Type() != tinygltf::NULL_TYPE) {
                    return v;
                }
            }
            
        } else if(val.IsObject()) {
            const auto& keys = val.Keys();
            for(auto ite = keys.begin(); ite != keys.end(); ++ite) {
                auto& k = *ite;
                if(k.compare(targetKey) == 0) {
                    return val.Get(targetKey);
                    
                } else {
                    auto& v = val.Get(k);
                    if(v.IsObject() || v.IsArray()) {
                        return FindSpectrenotesExtra(v, targetKey);
                    }
                }
            }
        }
        
        return tinygltf::Value();
    }
    
    std::string getSpectrenotesExtraKey(std::string key) {
        return kExtraPrefix + key;
    }
    
    // Parse functions
    int ParseTextures(const tinygltf::Model& model, AssetLibrary *assetlib) {
        int count = 0;
        if(model.textures.size() <= 0) {
            return 0;
        }
        assetlib->textures.reserve(model.textures.size());
        
        for(auto ite = model.textures.begin(); ite != model.textures.end(); ++ite) {
            const tinygltf::Texture& gltftex = *ite;
            const tinygltf::Image& gltfimg = model.images.at(gltftex.source);
            
            double gamma = 2.2;
            auto ppextra = FindSpectrenotesExtra(gltfimg.extras, getSpectrenotesExtraKey("gamma"));
            if (ppextra.Type() != tinygltf::NULL_TYPE) {
                gamma = ppextra.IsInt() ? ppextra.Get<int>() : ppextra.Get<double>();
            }
            
            ImageTexture *imgtex = new ImageTexture(gltfimg.width, gltfimg.height);
            imgtex->name = gltfimg.name;
            
            // Bitmap
            switch (gltfimg.bits) {
                case 8:
                    imgtex->initWith8BPPImage(gltfimg.image.data(), gltfimg.component, gamma);
                    break;
                case 16:
                    imgtex->initWith16BPPImage((const unsigned short*)gltfimg.image.data(), gltfimg.component, gamma);
                    break;
                default:
                    std::cerr << "texture:" << gltftex.name;
                    std::cerr << ", src:" << gltfimg.name << "(" << gltfimg.uri << ")";
                    std::cerr << " is unsupported bits per pixels value:" << gltfimg.bits;
                    std::cerr << std::endl;
                    
                    imgtex->fillColor(Color(1.0, 0.0, 1.0), 1.0, gamma);
                    break;
            }
            
            // Mag filter
            if(gltftex.sampler >= 0) {
                const tinygltf::Sampler& gltfsampler = model.samplers.at(gltftex.sampler);
                
                switch (gltfsampler.magFilter) {
                    case TINYGLTF_TEXTURE_FILTER_NEAREST:
                        imgtex->sampleType = ImageTexture::kNearest;
                        break;
                    case TINYGLTF_TEXTURE_FILTER_LINEAR:
                        imgtex->sampleType = ImageTexture::kLinear;
                        break;
                    default:
                        imgtex->sampleType = ImageTexture::kLinear;
                        break;
                }
                
                // TODO? Min filter is skip now
                
                // Wrap x
                switch (gltfsampler.wrapS) {
                    case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
                        imgtex->wrapX = ImageTexture::kClamp;
                        break;
                    case TINYGLTF_TEXTURE_WRAP_REPEAT:
                        imgtex->wrapX = ImageTexture::kRepeat;
                        break;
                    default:
                        std::cerr << "texture:" << gltftex.name;
                        std::cerr << ", src:" << gltfimg.name << "(" << gltfimg.uri << ")";
                        std::cerr << " is using unsupported wrapS:" << gltfsampler.wrapS;
                        std::cerr << ". now using clamp.";
                        std::cerr << std::endl;
                        
                        imgtex->wrapX = ImageTexture::kClamp;
                        break;
                }
                
                // Wrap y
                switch (gltfsampler.wrapT) {
                    case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
                        imgtex->wrapY = ImageTexture::kClamp;
                        break;
                    case TINYGLTF_TEXTURE_WRAP_REPEAT:
                        imgtex->wrapY = ImageTexture::kRepeat;
                        break;
                    default:
                        std::cerr << "texture:" << gltftex.name;
                        std::cerr << ", src:" << gltfimg.name << "(" << gltfimg.uri << ")";
                        std::cerr << " is using unsupported wrapT:" << gltfsampler.wrapT;
                        std::cerr << ". now using clamp.";
                        std::cerr << std::endl;
                        
                        imgtex->wrapY = ImageTexture::kClamp;
                        break;
                }
            }
            
            // Add
            assetlib->textures.push_back(std::shared_ptr<Texture>(imgtex));
            count += 1;
        }
        return count;
    }
    
    int ParseMaterials(const tinygltf::Model& model, AssetLibrary *assetlib) {
        int count = 0;
        if(model.materials.size() <= 0) {
            return 0;
        }
        assetlib->materials.reserve(model.materials.size());
        
        for(auto ite = model.materials.begin(); ite != model.materials.end(); ++ite) {
            const tinygltf::Material& gltfmat = *ite;
            
            Material *mat = new Material();
            mat->name = gltfmat.name;
            
            // "pbrMetallicRoughness"
            for(auto valite = gltfmat.values.begin(); valite != gltfmat.values.end(); ++valite) {
                auto &kv = *valite;
                std::string key = kv.first;
                tinygltf::Parameter val = kv.second;

                // std::cerr << key << " found" << std::endl;
                
                if(key.compare("baseColorTexture") == 0) {
                    mat->baseColorTexture.texCoord = val.TextureTexCoord();
                    mat->baseColorTexture.texture = assetlib->textures[val.TextureIndex()].get();
                    
                } else if(key.compare("baseColorFactor") == 0) {
                    const auto& col = val.ColorFactor();
                    mat->baseColorFactor = Color(col.at(0), col.at(1), col.at(2));
                    mat->baseColorAlpha = col.at(3);
                    
                } else if(key.compare("metallicRoughnessTexture") == 0) {
                    mat->metallicRoughnessTexture.texCoord = val.TextureTexCoord();
                    mat->metallicRoughnessTexture.texture = assetlib->textures[val.TextureIndex()].get();
                    
                } else if(key.compare("metallicFactor") == 0) {
                    mat->metallicFactor = val.number_value;
                    
                } else if(key.compare("roughnessFactor") == 0) {
                    mat->roughnessFactor = val.number_value;
                    
                }
            }

            // additional
            for(auto valite = gltfmat.additionalValues.begin(); valite != gltfmat.additionalValues.end(); ++valite) {
                auto& kv = *valite;
                std::string key = kv.first;
                tinygltf::Parameter val = kv.second;

                // std::cerr << key << " found" << std::endl;
                
                if(key.compare("normalTexture") == 0) {
                    mat->normalTexture.texCoord = val.TextureTexCoord();
                    mat->normalTexture.texture = assetlib->textures[val.TextureIndex()].get();
                    
                } else if(key.compare("occlusionTexture") == 0) {
                    mat->occlusionTexture.texCoord = val.TextureTexCoord();
                    mat->occlusionTexture.texture = assetlib->textures[val.TextureIndex()].get();
                    
                } else if(key.compare("emissiveTexture") == 0) {
                    mat->emissiveTexture.texCoord = val.TextureTexCoord();
                    mat->emissiveTexture.texture = assetlib->textures[val.TextureIndex()].get();
                    
                } else if(key.compare("emissiveFactor") == 0) {
                    const auto& col = val.ColorFactor();
                    mat->emissiveFactor = Color(col.at(0), col.at(1), col.at(2));
                    
                } else if(key.compare("alphaMode") == 0) {
                    const auto& amode = val.string_value;
                    if(amode.compare("OPAQUE") == 0) {
                        mat->alphaMode = Material::kAlphaAsOpaque;
                    } else if(amode.compare("MASK") == 0) {
                        mat->alphaMode = Material::kAlphaAsMask;
                    } else if(amode.compare("BLEND") == 0) {
                        mat->alphaMode = Material::kAlphaAsBlend;
                    } else {
                        std::cerr << "Unknown alphaMode:" << amode << std::endl;
                        mat->alphaMode = Material::kAlphaAsBlend;
                    }
                    
                } else if(key.compare("alphaCutoff") == 0) {
                    mat->alphaCutoff = val.number_value;
                    
                } else if(key.compare("doubleSided") == 0) {
                    mat->doubleSided = val.bool_value;
                }
            }
            
            // extra
            auto ppextra = FindSpectrenotesExtra(gltfmat.extras, getSpectrenotesExtraKey("ior"));
            if (ppextra.Type() != tinygltf::NULL_TYPE) {
                mat->ior = ppextra.Get<RTFloat>();
            }
            
            // Add
            mat->assetId = count;
            assetlib->materials.push_back(std::shared_ptr<Material>(mat));
            count += 1;
        }
        return count;
    }
    
    int ParseMeshs(const tinygltf::Model& model, AssetLibrary *assetlib) {
        int count = 0;
        if(model.meshes.size() <= 0) {
            return 0;
        }
        assetlib->meshes.reserve(model.meshes.size());
        
        for(auto ite = model.meshes.begin(); ite != model.meshes.end(); ++ite) {
            auto& gltfmesh = *ite;
            auto& gltfprims = gltfmesh.primitives;
            
            if(gltfprims.size() <= 0) {
                continue;
            }
            
            auto* mesh = new Mesh();
            mesh->clusters.reserve(gltfprims.size());
            
            int primCount = 0;
            int totalVerts = 0;
            int totalTris = 0;
            for(auto prmite = gltfprims.begin(); prmite != gltfprims.end(); ++prmite) {
                auto& gltfprimitive = *prmite;
                
                // Vertex
                int accid;
                BufferAccessor posba;
                BufferAccessor nrmba;
                BufferAccessor tanba;

                std::map<Mesh::AttributeId, int> attrMap;
                int numUvs = 0;
                int numJoints = 0;
                int numWeights = 0;
                int numColors = 0;
                for (auto kv : gltfprimitive.attributes) {
                    auto& key = kv.first;

                    if (key.compare("POSITION") == 0) {
                        accid = gltfprimitive.attributes.at("POSITION");
                        posba.init(model.accessors[accid], model);
                    } else if (key.compare("NORMAL") == 0) {
                        accid = gltfprimitive.attributes.at("NORMAL");
                        nrmba.init(model.accessors[accid], model);
                        attrMap[Mesh::AttributeId::kNormal] = 1;
                    } else if (key.compare("TANGENT") == 0) {
                        accid = gltfprimitive.attributes.at("TANGENT");
                        tanba.init(model.accessors[accid], model);
                        attrMap[Mesh::AttributeId::kTangent] = 1;
                    }
                    else if (key.find("TEXCOORD") == 0) {
                        numUvs += 1;
                    } else if (key.find("JOINTS") == 0) {
                        numJoints += 1;
                    } else if (key.find("WEIGHTS") == 0) {
                        numWeights += 1;
                    } else if (key.find("COLOR") == 0) {
                        numColors += 1;
                    }
                }

                attrMap[Mesh::AttributeId::kUv] = numUvs;
                attrMap[Mesh::AttributeId::kJoints] = numJoints;
                attrMap[Mesh::AttributeId::kWeights] = numWeights;
                attrMap[Mesh::AttributeId::kColor] = numColors;

                std::vector<BufferAccessor> uvba(numUvs);
                std::vector<BufferAccessor> jointba(numJoints);
                std::vector<BufferAccessor> weightba(numWeights);
                std::vector<BufferAccessor> colorba(numColors);
                std::stringstream attrNameSS;

                std::vector<BufferAccessor>* batbl[] = {
                    &uvba,
                    &jointba,
                    &weightba,
                    &colorba
                };
                std::string attrtbl[] = {
                    "TEXCOORD_",
                    "JOINTS_",
                    "WEIGHTS_",
                    "COLOR_"
                };

                for (int iba = 0; iba < 4; iba++) {
                    auto* baptr = batbl[iba];
                    for (size_t iattr = 0; iattr < baptr->size(); iattr++) {
                        attrNameSS.str("");
                        attrNameSS << attrtbl[iba] << iattr;
                        std::string key = attrNameSS.str();

                        if (gltfprimitive.attributes.find(key) == gltfprimitive.attributes.end()) {
                            continue;
                        }
                        accid = gltfprimitive.attributes.at(key);
                        baptr->at(iattr).init(model.accessors[accid], model);
                    }
                }
                
                // Triangle
                int numverts = posba.getStructCount();

                BufferAccessor indba;
                std::vector<int> tmpindice;

                if (gltfprimitive.indices >= 0) {
                    indba.init(model.accessors[gltfprimitive.indices], model);
                } else {
                    const char* errmodes[] = {
                        "POINT", "LINES", "LINE_LOOP", "LINE_STRIP"
                    };

                    switch (gltfprimitive.mode)
                    {
                    case TINYGLTF_MODE_TRIANGLES:
                        tmpindice.resize(numverts);
                        for (size_t iind = 0; iind < tmpindice.size(); iind++) {
                            tmpindice[iind] = static_cast<int>(iind);
                        }
                        break;
                    case TINYGLTF_MODE_TRIANGLE_STRIP:
                    {
                        int numtris = numverts - 2;
                        tmpindice.resize(numtris * 3);
                        const int indoffsets[2][3] = { {0, 1, 2}, {2, 0, 1} };
                        for (int itri = 0; itri < numtris; itri ++) {
                            int iind = itri * 3;
                            const int* ofst = indoffsets[itri & 1];
                            tmpindice[iind + ofst[0]] = itri;
                            tmpindice[iind + ofst[1]] = itri + 1;
                            tmpindice[iind + ofst[2]] = itri + 2;
                        }
                    }
                        break;
                    case TINYGLTF_MODE_TRIANGLE_FAN:
                    {
                        int numtris = (numverts - 1) / 2;
                        tmpindice.resize(numtris * 3);
                        for (int itri = 0; itri < numtris; itri++) {
                            int iind = itri * 3;
                            tmpindice[iind] = 0;
                            tmpindice[iind] = itri + 1;
                            tmpindice[iind] = itri + 2;
                        }
                    }
                        break;
                    default:
                        std::cerr << "glTF primitive mode " << errmodes[gltfprimitive.mode] << "(" << gltfprimitive.mode << ") not supported." << std::endl;
                        tmpindice.resize(3);
                        tmpindice[0] = 0;
                        tmpindice[1] = 0;
                        tmpindice[2] = 0;
                        break;
                    }

                    indba.initWithData<int>(tmpindice.data(), static_cast<int>(tmpindice.size()), 1);
                }

                int numtris = indba.getStructCount() / 3;
                auto* cluster = new Mesh::Cluster(numverts, numtris, attrMap);
                
                for(int iv = 0; iv < numverts; iv++) {
                    Vector3 &v = cluster->vertices[iv];
                    v.x = posba.readFloat();
                    v.y = posba.readFloat();
                    v.z = posba.readFloat();
                    
                    Attributes attrs = cluster->attributesAt(iv);
                    attrs.normal->x = nrmba.readFloat();
                    attrs.normal->y = nrmba.readFloat();
                    attrs.normal->z = nrmba.readFloat();
                    
                    attrs.tangent->x = tanba.readFloat();
                    attrs.tangent->y = tanba.readFloat();
                    attrs.tangent->z = tanba.readFloat();
                    attrs.tangent->w = tanba.readFloat();
                    
                    for (int j = 0; j < numUvs; j++) {
                        Vector3* uvn = attrs.uv0 + j;
                        auto& ba = uvba[j];
                        uvn->x = ba.readFloat();
                        uvn->y = ba.readFloat();
                        uvn->z = 0.0;
                    }

                    for (int j = 0; j < numColors; j++) {
                        Vector4* colorn = attrs.color0 + j;
                        auto& ba = colorba[j];
                        colorn->x = ba.readFloat();
                        colorn->y = ba.readFloat();
                        colorn->z = ba.readFloat();
                        if (ba.getComponentCountInStruct() > 3) {
                            colorn->w = ba.readFloat();
                        } else {
                            colorn->w = 0.0;
                        }
                    }

                    for (int j = 0; j < numJoints; j++) {
                        IntVec4* jointsn = attrs.joints0 + j;
                        auto& ba = jointba[j];
                        jointsn->x = ba.readInt();
                        jointsn->y = ba.readInt();
                        jointsn->z = ba.readInt();
                        jointsn->w = ba.readInt();
                    }

                    for (int j = 0; j < numWeights; j++) {
                        Vector4* weightsn = attrs.weights0 + j;
                        auto& ba = weightba[j];
                        weightsn->x = ba.readFloat();
                        weightsn->y = ba.readFloat();
                        weightsn->z = ba.readFloat();
                        weightsn->w = ba.readFloat();
                    }
                }
                
                // Triangle
                for(int itri = 0; itri < numtris; itri++) {
                    Mesh::Triangle& tri = cluster->triangles[itri];
                    tri.a = indba.readInt();
                    tri.b = indba.readInt();
                    tri.c = indba.readInt();
                    tri.clusterId = primCount;
                }
                
                // Material
                if (gltfprimitive.material < 0) {
                    cluster->material = nullptr; // FIXME use place holder material
                } else {
                    cluster->material = assetlib->getMaterial(gltfprimitive.material);

                    //
                    auto sptr = std::shared_ptr<Mesh::Cluster>(cluster);
                    mesh->clusters.push_back(sptr);
                    if (cluster->material->emissiveFactor.getMaxComponent() > 0.0) {
                        mesh->emissiveClusters.push_back(sptr);
                    }
                }
                
                primCount += 1;
                totalVerts += numverts;
                totalTris += numtris;
            }
            mesh->totalVertices = totalVerts;
            mesh->totalTriangles = totalTris;
            
            // Add
            mesh->assetId = count;
            assetlib->meshes.push_back(std::shared_ptr<Mesh>(mesh));
            count += 1;
        }
        return count;
    }

    int ParseSkins(const tinygltf::Model& model, AssetLibrary* assetLib) {
        if (model.skins.size() <= 0) {
            return 0;
        }
        assetLib->skins.reserve(model.skins.size());
        
        for(auto ite = model.skins.begin(); ite != model.skins.end(); ++ite) {
            auto& gltfskin = *ite;
            auto* skin = new Skin();
            
            if(gltfskin.name.size() > 0) {
                skin->name = gltfskin.name;
            }
            
            int jointcount = static_cast<int>(gltfskin.joints.size());
            skin->jointNodes.reserve(jointcount);
            for(auto jntite = gltfskin.joints.begin(); jntite != gltfskin.joints.end(); ++jntite) {
                skin->jointNodes.push_back(assetLib->nodes[*jntite].get());
            }
            
            if(gltfskin.skeleton > 0) {
                skin->skeltonRoot = assetLib->nodes[gltfskin.skeleton].get();
            } else {
                skin->skeltonRoot = skin->jointNodes[0];
            }
            
            skin->inverseBindMatrices.resize(jointcount);
            if(gltfskin.inverseBindMatrices >= 0) {
                BufferAccessor ibmba;
                ibmba.init(model.accessors[gltfskin.inverseBindMatrices], model);
                
                for(int ijnt = 0; ijnt < jointcount; ijnt++) {
                    for(int imtx = 0; imtx < 16; imtx++) {
                        skin->inverseBindMatrices[ijnt].m[imtx] = ibmba.readFloat();
                    }
                }
            } else {
                for(int ijnt = 0; ijnt < jointcount; ijnt++) {
                    skin->inverseBindMatrices[ijnt].setIdentity();
                }
            }
            
            assetLib->skins.push_back(std::shared_ptr<Skin>(skin));
        }
        
        // resolve skins
        for(auto ite = skinedMeshNodes->begin(); ite != skinedMeshNodes->end(); ++ite) {
            auto& skinmesh = *ite;
            skinmesh.node->content.skin = assetLib->skins[skinmesh.skinIndex].get();
        }

        return static_cast<int>(assetLib->skins.size());
    }

    int ParseAnimations(const tinygltf::Model& model, AssetLibrary* assetLib) {
        if(model.animations.size() <= 0) {
            return 0;
        }
        assetLib->animations.reserve(model.animations.size());
        
        for(auto ite = model.animations.begin(); ite != model.animations.end(); ++ite) {
            auto& gltfanim = *ite;
            auto* anim = new Animation();
            
            if(gltfanim.name.size() > 0) {
                anim->name = gltfanim.name;
            }
            
            // samplers
            anim->samplers.reserve(gltfanim.samplers.size());
            for(auto smpite = gltfanim.samplers.begin(); smpite != gltfanim.samplers.end(); ++ smpite) {
                auto& gltfsampler = *smpite;
                auto* kfsampler = new KeyframeSampler();
                
                BufferAccessor inputba(model.accessors[gltfsampler.input], model);
                BufferAccessor outputba(model.accessors[gltfsampler.output], model);
                
                int keycount = inputba.getTotalComponentCount();
                kfsampler->timeStamps.resize(keycount);
                for(int ikey = 0; ikey < keycount; ikey++) {
                    kfsampler->timeStamps[ikey] = static_cast<RTTimeType>(inputba.readFloat());
                }
                
                int smplcount = outputba.getTotalComponentCount();
                kfsampler->sampleBuffer.resize(smplcount);
                for(int ismp = 0; ismp < smplcount; ismp++) {
                    kfsampler->sampleBuffer[ismp] = outputba.readFloat();
                }
                kfsampler->sampleComponents = outputba.getComponentCountInStruct();
                
                if(gltfsampler.interpolation.size() > 0) {
                    if(gltfsampler.interpolation.compare("STEP") == 0) {
                        kfsampler->interpolation = KeyframeSampler::kStep;
                    } else if(gltfsampler.interpolation.compare("CUBICSPLINE") == 0) {
                        kfsampler->interpolation = KeyframeSampler::kCubicSpline;
                    } else {
                        kfsampler->interpolation = KeyframeSampler::kLinear;
                    }
                }
                
                anim->samplers.push_back(std::shared_ptr<KeyframeSampler>(kfsampler));
            }
            
            // target channels
            size_t chcount = gltfanim.channels.size();
            anim->targets.resize(chcount);
            int ich = 0;
            for(auto chite = gltfanim.channels.begin(); chite != gltfanim.channels.end(); ++chite, ++ich) {
                auto& gltfch = *chite;
                auto& targetch = anim->targets[ich];
                
                targetch.sampler = anim->samplers[gltfch.sampler].get();
                targetch.node = assetLib->nodes[gltfch.target_node].get();
                const auto& chpath = gltfch.target_path;
                if(chpath.compare("translation") == 0) {
                    targetch.targetProp = Animation::kTranslation;
                } else if(chpath.compare("rotation") == 0) {
                    targetch.targetProp = Animation::kRotation;
                } else if(chpath.compare("scale") == 0) {
                    targetch.targetProp = Animation::kScale;
                } else if(chpath.compare("weights") == 0) {
                    targetch.targetProp = Animation::kMorphWeights;
                }

                targetch.node->animatedFlag = Node::kAnimatedDirect;
            }
            
            assetLib->animations.push_back(std::shared_ptr<Animation>(anim));
        }

        return static_cast<int>(assetLib->animations.size());
    }
    
    /*
    int ParseLights(const tinygltf::Model& model, AssetLibrary *assetlib) {
        int count = 0;
        for(auto ite = model.lights.begin(); ite != model.lights.end(); ++ite) {
            auto& gltflight = *ite;
            
            auto* lit = new Light();
            lit->name = gltflight.name;
            
            if(gltflight.color.size() > 0) {
                lit->color.set(gltflight.color[0], gltflight.color[1], gltflight.color[2]);
            }
            
            if (gltflight.type.compare("POINT") == 0) {
                lit->lightType = Light::kPointLight;
            }
            else if (gltflight.type.compare("SPOT") == 0) {
                lit->lightType = Light::kSpotLight;
            }
            else if (gltflight.type.compare("DIRECTIONAL") == 0) {
                lit->lightType = Light::kDirectionalLight;
            }
            
            assetlib->lights.push_back(std::shared_ptr<Light>(lit));
            count += 1;
        }
        return count;
    }
    */
    
    int ParseLights(const tinygltf::Model& model, AssetLibrary *assetlib) {
        int count = 0;
        if(model.extensions.find("KHR_lights_punctual") == model.extensions.end()) {
            return 0;
        }
        
        auto& khr_lights = model.extensions.at("KHR_lights_punctual");
        if(!khr_lights.Has("lights")) {
            return 0;
        }
        auto& lights = khr_lights.Get("lights");
        if(!lights.IsArray()) {
            return 0;
        }
        
        if(lights.ArrayLen() <= 0) {
            return 0;
        }
        assetlib->lights.reserve(lights.ArrayLen());
        
        for(int ilit = 0; ilit < lights.ArrayLen(); ilit++) {
            auto& lit_punc = lights.Get(ilit);
            if(!lit_punc.IsObject()) {
                continue;
            }
            
            auto* lit = new Light();
            
            if(lit_punc.Has("name")) {
                lit->name = lit_punc.Get("name").Get<std::string>();
            }
            
            if(lit_punc.Has("color")) {
                auto& col = lit_punc.Get("color");
                if(col.IsArray()) {
                    auto& r = col.Get(0);
                    auto& g = col.Get(1);
                    auto& b = col.Get(2);
                    lit->color.set(
                        r.IsInt()? r.Get<int>() : r.Get<double>(),
                        g.IsInt()? g.Get<int>() : g.Get<double>(),
                        b.IsInt()? b.Get<int>() : b.Get<double>()
                    );
                }
            }
            
            if(lit_punc.Has("intensity")) {
                auto& val = lit_punc.Get("intensity");
                lit->intensity = val.IsInt()? val.Get<int>() : val.Get<double>();
            }
            
            if(lit_punc.Has("type")) {
                std::string littype = lit_punc.Get("type").Get<std::string>();
                if(littype.compare("point") == 0) {
                    lit->lightType = Light::kPointLight;
                    
                } else if(littype.compare("directional") == 0) {
                    lit->lightType = Light::kPointLight;
                    
                } else if(littype.compare("sopt") == 0) {
                    lit->lightType = Light::kSpotLight;
                    
                } else {
                    std::cerr << "unknown light type:" << littype << std::endl;
                }
            }
            
            if(lit_punc.Has("spot")) {
                auto& spot = lit_punc.Get("spot");
                if(spot.IsObject()) {
                    if(spot.Has("innerConeAngle")) {
                        auto& val = spot.Get("innerConeAngle");
                        lit->spot.innerConeAngle = val.IsInt()? val.Get<int>() : val.Get<double>();
                    }
                    if(spot.Has("outerConeAngle")) {
                        auto& val = spot.Get("outerConeAngle");
                        lit->spot.innerConeAngle = val.IsInt()? val.Get<int>() : val.Get<double>();
                    }
                }
            }
            //
            assetlib->lights.push_back(std::shared_ptr<Light>(lit));
            count += 1;
        }
        return count;
    }
    
    int ParseCameras(const tinygltf::Model& model, AssetLibrary *assetlib) {
        int count = 0;
        if(model.cameras.size() <= 0) {
            return 0;
        }
        assetlib->cameras.reserve(model.cameras.size());
        
        for(auto ite = model.cameras.begin(); ite != model.cameras.end(); ++ite) {
            auto& gltfcam = *ite;
            
            Camera *cam = new Camera();
            cam->name = gltfcam.name;

            // DOF parameters needs before init
            tinygltf::Value ppextra;
            //ppextra = FindSpectrenotesExtra(gltfcam.extras, getSpectrenotesExtraKey("focalLength"));
            //if (ppextra.Type() != tinygltf::NULL_TYPE) {
            //    cam->focalLength = ppextra.Get<RTFloat>();
            //}
            ppextra = FindSpectrenotesExtra(gltfcam.extras, getSpectrenotesExtraKey("fNumber"));
            if (ppextra.Type() != tinygltf::NULL_TYPE) {
                cam->fNumber = ppextra.Get<RTFloat>();
            }
            ppextra = FindSpectrenotesExtra(gltfcam.extras, getSpectrenotesExtraKey("focusDistance"));
            if (ppextra.Type() != tinygltf::NULL_TYPE) {
                cam->focusDistance = ppextra.Get<RTFloat>();
            }
            
            if (gltfcam.type.compare("perspective") == 0) {
                cam->initWithType(Camera::kPerspectiveCamera);
                auto& gltfpers = gltfcam.perspective;
                cam->perspective.aspect = gltfpers.aspectRatio;
                cam->perspective.yfov = gltfpers.yfov;
                cam->perspective.zfar = gltfpers.zfar;
                cam->perspective.znear = gltfpers.znear;
                
            } else if (gltfcam.type.compare("orthographic") == 0) {
                cam->initWithType(Camera::kOrthographicsCamera);
                auto& gltfortho = cam->orthographics;
                cam->orthographics.xmag = gltfortho.xmag;
                cam->orthographics.ymag = gltfortho.ymag;
                cam->orthographics.zfar = gltfortho.zfar;
                cam->orthographics.znear = gltfortho.znear;
                
            } else {
                std::cerr << "Unknown type camera found. skip:" << gltfcam.type << std::endl;
                continue;
            }
            
            //
            assetlib->cameras.push_back(std::shared_ptr<Camera>(cam));
            count += 1;
        }
        return count;
    }
    
    int ParseNodes(const tinygltf::Model& model, AssetLibrary *assetlib) {
        int count = 0;
        if(model.nodes.size() <= 0) {
            return 0;
        }
        assetlib->nodes.reserve(model.nodes.size());
        
        for(auto ite = model.nodes.begin(); ite != model.nodes.end(); ++ite) {
            auto& gltfnode = *ite;
            
            auto* node = new Node(count);
            node->name = gltfnode.name;
            
            // get transform
            bool hastrs = false;
            bool hasmatrix = false;
            
            if(gltfnode.translation.size() > 0) {
                hastrs = true;
                node->initialTransform.translate.set(
                    gltfnode.translation[0],
                    gltfnode.translation[1],
                    gltfnode.translation[2]
                );
            }
            
            if(gltfnode.rotation.size() > 0) {
                hastrs = true;
                node->initialTransform.rotation.set(
                    gltfnode.rotation[0],
                    gltfnode.rotation[1],
                    gltfnode.rotation[2],
                    gltfnode.rotation[3]
                );
            }
            
            if(gltfnode.scale.size() > 0) {
                hastrs = true;
                node->initialTransform.scale.set(
                    gltfnode.scale[0],
                    gltfnode.scale[1],
                    gltfnode.scale[2]
                );
            }
            
            if(gltfnode.matrix.size() > 0) {
                auto& mat = gltfnode.matrix;
                node->initialTransform.matrix.set(
                    mat[0], mat[1], mat[2], mat[3],
                    mat[4], mat[5], mat[6], mat[7],
                    mat[8], mat[9], mat[10], mat[11],
                    mat[12], mat[13], mat[14], mat[15]
                );
            }
            
            if(hastrs) {
                node->initialTransform.makeMatrix();
            }
            
            // node content
            if(gltfnode.camera >= 0) {
                node->contentType = Node::kContentTypeCamera;
                node->content.camera = assetlib->cameras[gltfnode.camera].get();
                
            } else if(gltfnode.mesh >= 0) {
                node->contentType = Node::kContentTypeMesh;
                node->content.mesh = assetlib->meshes[gltfnode.mesh].get();
                node->content.skin = nullptr;
                // skin is not parsed here.
                if (gltfnode.skin >= 0) {
                    SkinedMeshNode smn;
                    smn.node = node;
                    smn.skinIndex = gltfnode.skin;
                    skinedMeshNodes->push_back(smn);
                }
            }
            
            size_t numchild = gltfnode.children.size();
            if(numchild > 0) {
                node->children.resize(numchild);
                for(int ichld = 0; ichld < numchild; ichld++) {
                    node->children[ichld] = gltfnode.children[ichld];
                }
            }
            
            auto& extensions = gltfnode.extensions;
            if(extensions.find("KHR_lights_punctual") != extensions.end()) {
                auto& lit_punk = extensions.at("KHR_lights_punctual");
                if(lit_punk.IsObject()) {
                    auto& litid = lit_punk.Get("light");
                    if(litid.IsInt()) {
                        node->content.light = assetlib->lights[litid.Get<int>()].get();
                        node->contentType = Node::kContentTypeLight;
                    }
                }
            }
            
            //
            assetlib->nodes.push_back(std::shared_ptr<Node>(node));
            
            count += 1;
        }
        return count;
    }
    
    int ParseScenes(const tinygltf::Model& model, AssetLibrary *assetlib) {
        int count = 0;
        if(model.scenes.size() <= 0) {
            return 0;
        }
        assetlib->scenes.reserve(model.scenes.size());
        
        for(auto ite = model.scenes.begin(); ite != model.scenes.end(); ++ite) {
            auto& gltfscene = *ite;
            
            auto* scn = new Scene(assetlib);
            if(gltfscene.nodes.size() > 0) {
                // scene top level nodes
                scn->topLevelNodes.reserve(gltfscene.nodes.size());
                for(auto nodeite = gltfscene.nodes.begin(); nodeite != gltfscene.nodes.end(); ++nodeite) {
                    int nodeid = *nodeite;
                    auto* node = assetlib->nodes[nodeid].get();
                    scn->topLevelNodes.push_back(node);
                }
            }

            auto ppextra = FindSpectrenotesExtra(gltfscene.extras, getSpectrenotesExtraKey("background"));
            if (ppextra.Type() != tinygltf::NULL_TYPE) {
                auto bgimg = ppextra.Get<std::string>();
                bgImagePaths->insert(std::pair(count, bgimg));
            }
            
            assetlib->scenes.push_back(std::shared_ptr<Scene>(scn));
            count += 1;
        }
        return count;
    }
    
    void LoadBackgroundTexture(const std::string path, AssetLibrary *assetlib) {
        if(path.length() > 0) {
            auto tex = ImageTexture::loadImageFile(path);
            assetlib->backgroundTex = std::shared_ptr<Texture>(tex);
        } else {
            const int w = 1024;
            const int h = 512;
            
            std::vector<float> buf(w * h * 4);
            float* data = buf.data();

            auto fract = [](float x) { return x - std::floor(x); };
            const Vector3 azimcolTbl[8] = {
                Vector3(0.2, 0.2, 0.5), // -z
                Vector3(0.5, 0.2, 0.2), // -x
                Vector3(0.6, 0.3, 0.3), // -x
                Vector3(0.2, 0.2, 0.7), // +z
                Vector3(0.3, 0.3, 0.9), // +z
                Vector3(0.9, 0.3, 0.3), // +x
                Vector3(0.7, 0.2, 0.2), // +x
                Vector3(0.3, 0.3, 0.6)  // -z
            };
            const int aimcolNum = sizeof(azimcolTbl) / sizeof(azimcolTbl[0]);

            for (int iy = 0; iy < h; iy++) {
                float ty = static_cast<float>(iy) / h;
                for (int ix = 0; ix < w; ix++) {
                    float tx = static_cast<float>(ix) / w;
                    float* pxl = data + (ix + iy * w) * 4;

                    Vector3 c;
                    bool checker = (fract(tx * 36.0f) > 0.5f) ^ (fract(ty * 18.0f) > 0.5f);
                    c = (checker ? 0.5 : 1.0) * azimcolTbl[static_cast<int>(tx * 8.0f)];
                    //c = azimcolTbl[static_cast<int>(tx * 8.0f)];
                    //c = azimcolTbl[ix % aimcolNum];

                    if (iy < h / 4) {
                        c = Vector3::mul(c, Vector3(0.5, 1.0, 0.5));
                    }
                    else if (iy > h * 3 / 4) {
                        c = Vector3::mul(c, Vector3(0.2, 0.8, 0.2));
                    }
                    else if (iy > h / 2) {
                        c = Vector3::mul(c, Vector3(0.5, 0.5, 0.5));
                    }

                    if (iy < h / 8) {
                        c += Vector3(0.8, 0.8, 0.8);
                    }

                    if ((ix % (w / 8)) < 1 || ((iy % (h / 8)) < 1)) {
                        c.set(0.02, 0.02, 0.02);
                    }
                    if (((ix + 1) % (w / 2)) < 2 || (((iy + 1) % (h / 2)) < 2)) {
                        c.set(0.2, 0.02, 0.02);
                    }

                    pxl[0] = static_cast<float>(c.x);
                    pxl[1] = static_cast<float>(c.y);
                    pxl[2] = static_cast<float>(c.z);
                    pxl[3] = 1.0f;
                }
            }
            
            auto* tex = new ImageTexture(w, h);
            tex->initWithFpImage(data, 4, 1.0);
            assetlib->backgroundTex = std::shared_ptr<Texture>(tex);
        }
    }
}

AssetLibrary* SceneLoader::loadGLTF(std::string filepath) {
    tinygltf::TinyGLTF gltfloader;
    
    tinygltf::Model model;
    std::string err;
    std::string warn;
    bool loadres;
    
    loadres = gltfloader.LoadASCIIFromFile(&model, &err, &warn, filepath);
    
    if(!loadres) {
        std::cerr << "glTF load err:" << err << std::endl;
        return nullptr;
    }
    
    AssetLibrary *assetLib = new AssetLibrary();
    skinedMeshNodes = new std::vector<SkinedMeshNode>();
    bgImagePaths = new std::map<int, std::string>();
    
    ParseTextures(model, assetLib);
    ParseMaterials(model, assetLib);
    ParseMeshs(model, assetLib);
    ParseLights(model, assetLib);
    ParseCameras(model, assetLib);
    ParseNodes(model, assetLib);
    ParseSkins(model, assetLib);
    ParseAnimations(model, assetLib);
    ParseScenes(model, assetLib);
    
    assetLib->defaultSceneId = model.defaultScene;

    if (bgImagePaths->find(assetLib->defaultSceneId) != bgImagePaths->end()) {
        LoadBackgroundTexture(bgImagePaths->at(assetLib->defaultSceneId), assetLib);
    } else {
        LoadBackgroundTexture(std::string(), assetLib);
    }
    
    delete bgImagePaths;
    delete skinedMeshNodes;
    return assetLib;
}
