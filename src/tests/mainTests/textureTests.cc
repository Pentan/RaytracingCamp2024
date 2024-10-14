
#include <cmath>
#include <iostream>
#include <sstream>
#include <doctest.h>
#include "../testsupport.h"

#include <petals/types.h>
#include <petals/texture.h>

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

using namespace Petals;

namespace {
    void SaveImage(std::string filename, const ImageTexture* tex, const double gamma=2.2) {
        std::string outdir = "textureTest";
        CheckTestOutputDir(outdir);

        int w = tex->width;
        int h = tex->height;
        double ungamma = 1.0 / gamma;

        std::vector<unsigned char> buf(w * h * 4);
        unsigned char* data = buf.data();
        for (int iy = 0; iy < h; iy++) {
            RTFloat ty = iy / static_cast<RTFloat>(h);
            for (int ix = 0; ix < w; ix++) {
                RTFloat tx = ix / static_cast<RTFloat>(w);
                int i = (ix + iy * w) * 4;
                auto s = tex->sample(tx, ty, false);
                s.powRGB(ungamma);
                data[i + 0] = static_cast<unsigned char>(std::max(0.0, std::min(255.0, s.rgb.r * 256.0)));
                data[i + 1] = static_cast<unsigned char>(std::max(0.0, std::min(255.0, s.rgb.g * 256.0)));
                data[i + 2] = static_cast<unsigned char>(std::max(0.0, std::min(255.0, s.rgb.b * 256.0)));
                data[i + 3] = static_cast<unsigned char>(std::max(0.0, std::min(255.0, s.a * 256.0)));
            }
        }

        std::stringstream ss;
        ss << PETALS_TEST_OUTPUT_DIR << "/" << outdir << "/" << filename << ".png";
        std::string outpath = ss.str();

        int res = stbi_write_png(outpath.c_str(), w, h, 4, buf.data(), 0);
        if (res == 0) {
            std::cerr << "stbi_write_png failed" << std::endl;
        }
    }
}

TEST_CASE("ImageTexture test [Texture]") {
    const int w = 512;
    const int h = w / 2;

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

            if(iy < h / 4) {
                c = Vector3::mul(c, Vector3(0.5, 1.0, 0.5));
            } else if (iy > h * 3 / 4) {
                c = Vector3::mul(c, Vector3(0.2, 0.8, 0.2));
            } else if(iy > h / 2) {
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

    ImageTexture tex(w, h);
    tex.setWrap(ImageTexture::kClamp);
    tex.initWithFpImage(data, 4, 1.0);

    SaveImage("bgtest", &tex);

    REQUIRE(true);
}

TEST_CASE("ImageTexture load test [Texture]") {
    std::stringstream ss;
    //ss << PETALS_TEST_DATA_DIR << "/" << "normaltest0.png";
    ss << PETALS_TEST_DATA_DIR << "/" << "images/image01.tga";
    std::string path = ss.str();
    
    int x, y, c;
    auto* data = stbi_load(path.c_str(), &x, &y, &c, 0);
    REQUIRE(data != nullptr);

    ImageTexture tex(x, y);
    tex.initWith8BPPImage(data, c, 2.2);
    
    SaveImage("loadtest", &tex);
    
    REQUIRE(true);
}
