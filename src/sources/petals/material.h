#ifndef PETALS_METARIAL_H
#define PETALS_METARIAL_H

#include <string>
#include <vector>
#include <memory>

#include "types.h"
#include "ray.h"
#include "random.h"

namespace Petals {
    
    class Texture;
    
    //
    class Sampler {
    public:
        struct SampleResult {
            Vector3 v;
            RTFloat pdf;
        };
        static void basisFromNormal(const Vector3& n, Vector3* ou, Vector3* ov);
        static SampleResult sampleUniformHemisphere(const Vector3& n, Random& rng);
        static SampleResult sampleCosineWeightedHemisphere(const Vector3& n, Random& rng);
    };
    
    //
    class Material {
    public:
        enum AlphaMode {
            kAlphaAsOpaque,
            kAlphaAsMask,
            kAlphaAsBlend
        };
        
        struct TextureInfo {
            Texture *texture;
            int texCoord;
            RTFloat scale; // for normal and occulution
            
            TextureInfo():
                texture(nullptr),
                texCoord(0),
                scale(1.0)
            {}
        };
        
        enum BXDFType {
            kDiffuse,
            kSpecular,
            kTransmit,
            kEmission
        };
        
        struct EvalLog {
            RTFloat pdf;
            RTFloat bxdfPdf;
            RTFloat samplePdf;
            RTFloat bxdfValue;
            Color filterColor;
            int selectedBxdfId;
            BXDFType bxdfType;
        };
        
    public:
        Material();
        ~Material();
        
        std::string name;
        int assetId;
        
        Color evaluateThroughput(const Ray& iray, Ray* oray, const SurfaceInfo& surfinfo, Random& rng, EvalLog* log) const;
        RTFloat evaluateBXDF(const Ray& iray, const Ray& oray, int bxdfId, const SurfaceInfo& surfinfo, EvalLog* log) const;
        
        Color evaluateEmissive(const Vector3* uv0) const;
        Color evaluateAlbedoColor(const Vector3* uv0) const;
        Vector3 evaluateMetallicRoughness(const Vector3* uv0) const; // x:metallic, y:roughness
        Vector3 evaluateNormal(const Vector3* uv0, const Vector3& nrmv, const Vector4& tanv) const;
        
        // pbrMetallicRoughness
        TextureInfo baseColorTexture;
        Color baseColorFactor;
        RTColorType baseColorAlpha;
        TextureInfo metallicRoughnessTexture;
        RTFloat metallicFactor;
        RTFloat roughnessFactor;
        
        // Additions
        TextureInfo normalTexture;
        TextureInfo occlusionTexture;
        TextureInfo emissiveTexture;
        Color emissiveFactor;
        
        AlphaMode alphaMode;
        RTFloat alphaCutoff;
        bool doubleSided;
        
        // Extra
        RTFloat ior;
        // Clear coat factor
        // Shean factor
        // SSS factor
        // ...
    };
}

#endif
