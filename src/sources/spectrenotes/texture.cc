//
//  texture.cpp
//  PinkyCore
//
//  Created by SatoruNAKAJIMA on 2019/08/16.
//
#include <iostream>
#include <stb/stb_image.h>

#include "texture.h"
#include "pptypes.h"

using namespace PinkyPi;

// ImageTexture methods

ImageTexture::ImageTexture(int w, int h):
    width(w),
    height(h),
    hasAlpha(false),
    gamma(2.2),
    sampleType(kLinear),
    wrapX(kRepeat),
    wrapY(kRepeat)
{
    image = new TexcelSample[w * h];
}

ImageTexture::~ImageTexture() {
    delete [] image;
}

TexcelSample ImageTexture::sample(PPFloat x, PPFloat y, bool gammacorrect) const {
    TexcelSample ret;
    
    x *= width;
    y *= height;
    int ix = static_cast<int>(std::floor(x));
    int iy = static_cast<int>(std::floor(y));
    PPColorType tx = x - ix;
    PPColorType ty = y - iy;
    
    ix = wrapSampleX(ix);
    iy = wrapSampleY(iy);
    
    switch (sampleType) {
        case kNearest:
            ret = image[ix + iy * width];
            break;
        case kLinear:
        default:
        {
            int ix1 = wrapSampleX(ix + 1);
            int iy1 = wrapSampleY(iy + 1);
            TexcelSample s00 = image[ix  + iy * width];
            TexcelSample s10 = image[ix1 + iy * width];
            TexcelSample s01 = image[ix  + iy1 * width];
            TexcelSample s11 = image[ix1 + iy1 * width];
            
            PPColorType tx0 = 1.0 - tx;
            PPColorType tx1 = tx;
            
            TexcelSample s0010;
            s0010.rgb = s00.rgb * tx0 + s10.rgb * tx1;
            s0010.a = s00.a * tx0 + s10.a * tx1;
            TexcelSample s0111;
            s0111.rgb = s01.rgb * tx0 + s11.rgb * tx1;
            s0111.a = s01.a * tx0 + s11.a * tx1;
            
            PPColorType ty0 = 1.0 - ty;
            PPColorType ty1 = ty;
            ret.rgb = s0010.rgb * ty0 + s0111.rgb * ty1;
            ret.a = s0010.a * ty0 + s0111.a * ty1;
        }
            break;
    }

    if (gammacorrect) { ret.powRGB(gamma); }
    
    return ret;
}

void ImageTexture::fillColor(const Color rgb, PPColorType a, double gamma) {
    this->gamma = gamma;
    for(int i = 0; i < width * height; i++) {
        image[i].rgb = rgb;
        image[i].a = a;
        //image[i].powRGB(gamma);
    }
}

void ImageTexture::initWith8BPPImage(const unsigned char *src, int comps, double gamma) {
    this->gamma = gamma;
    for(int i = 0; i < width * height; i++) {
        TexcelSample& texel = image[i];
        int isrc = i * comps;
        texel.rgb.r = src[isrc] / 255.0;
        texel.rgb.g = (comps < 2) ? texel.rgb.r : src[isrc + 1] / 255.0;
        texel.rgb.b = (comps < 3) ? texel.rgb.r : src[isrc + 2] / 255.0;
        texel.a = (comps < 4) ? 1.0 : src[isrc + 3] / 255.0;
        //texel.powRGB(gamma);
    }
    hasAlpha = (comps >= 4);
}

void ImageTexture::initWith16BPPImage(const unsigned short *src, int comps, double gamma) {
    this->gamma = gamma;
    for(int i = 0; i < width * height; i++) {
        TexcelSample& texel = image[i];
        int isrc = i * comps;
        texel.rgb.r = src[isrc] / 65535.0;
        texel.rgb.g = (comps < 2) ? texel.rgb.r : src[isrc + 1] / 65535.0;
        texel.rgb.b = (comps < 3) ? texel.rgb.r : src[isrc + 2] / 65535.0;
        texel.a = (comps < 4) ? 1.0 : src[isrc + 3] / 65535.0;
        //texel.powRGB(gamma);
    }
    hasAlpha = (comps >= 4);
}

void ImageTexture::initWithFpImage(const float *src, int comps, double gamma) {
    this->gamma = gamma;
    for(int i = 0; i < width * height; i++) {
        TexcelSample& texel = image[i];
        int isrc = i * comps;
        texel.rgb.r = src[isrc];
        texel.rgb.g = (comps < 2) ? texel.rgb.r : src[isrc + 1];
        texel.rgb.b = (comps < 3) ? texel.rgb.r : src[isrc + 2];
        texel.a = (comps < 4) ? 1.0 : src[isrc + 3];
        //texel.powRGB(gamma);
    }
    hasAlpha = (comps >= 4);
}

int ImageTexture::wrapSampleX(int x) const {
    switch (wrapX) {
        case kClamp:
            x = std::max(0, x);
            x = std::min(x, width - 1);
            break;
        case kRepeat:
        default:
            x = x % width;
            if(x < 0) x += width;
            break;
    }
    return x;
}

int ImageTexture::wrapSampleY(int y) const {
    switch (wrapY) {
        case kClamp:
            y = std::max(0, y);
            y = std::min(y, height - 1);
            break;
        case kRepeat:
        default:
            y = y % height;
            if(y < 0) y += height;
            break;
    }
    return y;
}

ImageTexture* ImageTexture::loadImageFile(std::string path) {
    int w, h, ch;
    float* data = stbi_loadf(path.c_str(), &w, &h, &ch, 0);
    if(data == nullptr) {
        std::cout  << "Image file load failed: " << path << std::endl;
        return nullptr;
    }
    
    auto img = new ImageTexture(w, h);
    img->initWithFpImage(data, ch, 1.0);
    
    stbi_image_free(data);
    
    return img;
}
