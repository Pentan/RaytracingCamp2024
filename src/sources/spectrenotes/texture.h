#ifndef PINKYPI_TEXTURE_H
#define PINKYPI_TEXTURE_H

#include <string>
#include <vector>
#include <memory>
#include <cmath>

#include <pinkycore/pptypes.h>

namespace PinkyPi {
    
    // sample
    struct TexcelSample {
        Color rgb;
        PPColorType a;
        
        TexcelSample():
            rgb(0.0),
            a(1.0)
        {}
        
        void powRGB(PPColorType x) {
            rgb.r = std::pow(rgb.r, x);
            rgb.g = std::pow(rgb.g, x);
            rgb.b = std::pow(rgb.b, x);
        }
    };
    
    // Base class
    class Texture {
    public:
        Texture() {};
        virtual ~Texture() {};
        
        // (0,0) to (1,1)
        virtual TexcelSample sample(PPFloat x, PPFloat y, bool gammacorrect) const = 0;

        TexcelSample sampleEquirectangular(const Vector3& v, bool gc) const {
            PPFloat theta = std::acos(v.y) / kPI;
            PPFloat phi = std::atan2(v.z, v.x) / kPI * 0.5 + 0.5;
            return sample(phi, theta, gc);
        }
        
        std::string name;
    };
    
    // Image texture
    class ImageTexture : public Texture {
    public:
        enum SampleType {
            kNearest,
            kLinear
        };
        enum WrapType {
            kClamp,
            kRepeat
        };
        
    public:
        ImageTexture(int w, int h);
        virtual ~ImageTexture();
        
        virtual TexcelSample sample(PPFloat x, PPFloat y, bool gammacorrect) const;
        
        void fillColor(const Color rgb, PPColorType a, double gamma);
        
        void initWith8BPPImage(const unsigned char *src, int comps, double gamma);
        void initWith16BPPImage(const unsigned short *src, int comps, double gamma);
        void initWithFpImage(const float *src, int comps, double gamma);
        
        static ImageTexture* loadImageFile(std::string path);
        
    public:
        int width;
        int height;
        bool hasAlpha;
        PPFloat gamma;
        
        SampleType sampleType;
        WrapType wrapX;
        WrapType wrapY;

        void setWrap(WrapType wx, WrapType wy) {
            wrapX = wx;
            wrapY = wy;
        }
        void setWrap(WrapType w) { setWrap(w, w); }
        
    private:
        TexcelSample *image;
        
        int wrapSampleX(int x) const;
        int wrapSampleY(int y) const;
    };
}

#endif
