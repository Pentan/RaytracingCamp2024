#include <iostream>
#include <sstream>

#include <doctest.h>
#include "../testsupport.h"

#include <pinkycore/pptypes.h>
#include <pinkycore/framebuffer.h>

#include <stb/stb_image_write.h>

using namespace PinkyPi;

namespace {
    void OutputImage(std::string filename, int w, int h, unsigned char *data) {
        std::string outdir = "framebufferTest";
        CheckTestOutputDir(outdir);
        
        std::stringstream ss;
        ss << PINKYPI_TEST_OUTPUT_DIR << "/" << outdir << "/" << filename << ".png";
        std::string outpath = ss.str();
        
        int res = stbi_write_png(outpath.c_str(), w, h, 3, data, 0);
        if(res == 0) {
            std::cerr << "stbi_write_png failed" << std::endl;
        }
    }
}

TEST_CASE("FrameBuffer basic test [FrameBuffer]") {
    FrameBuffer fb(300, 250, 64);
    
    REQUIRE(fb.getTileCols() == (int)std::ceil((double)fb.getWidth() / fb.getTileSize()));
    REQUIRE(fb.getTileRows() == (int)std::ceil((double)fb.getHeight() / fb.getTileSize()));
    
    int w = fb.getWidth();
    int h = fb.getHeight();
    
    for(int iy = 0; iy < h; iy++) {
        PPFloat ty = (PPFloat)iy / (h - 1.0);
        for(int ix = 0; ix < w; ix++) {
            PPFloat tx = (PPFloat)ix / (w - 1.0);
            fb.accumulate(ix, iy, Color(tx, ty, 0.0));
        }
    }
    
    Color c;
    c = fb.getColor(0, 0);
    REQUIRE((c.r == 0.0 && c.g == 0.0));
    c = fb.getColor(w - 1, 0);
    REQUIRE((c.r == 1.0 && c.g == 0.0));
    c = fb.getColor(0, h - 1);
    REQUIRE((c.r == 0.0 && c.g == 1.0));
    c = fb.getColor(w - 1, h - 1);
    REQUIRE((c.r == 1.0 && c.g == 1.0));
    
    unsigned char *data = new unsigned char[w * h * 3];
    int idata;
    
    // dump
    idata = 0;
    for(int iy = 0; iy < h; iy++) {
        for(int ix = 0; ix < w; ix++) {
            Color c = fb.getColor(ix, iy);
            data[idata] = (unsigned char)(c.r * 255);
            data[idata + 1] = (unsigned char)(c.g * 255);
            data[idata + 2] = (unsigned char)(c.b * 255);
            idata += 3;
        }
    }
    OutputImage("dump", w, h, data);
    
    // raw buffer dump
    idata = 0;
    for(int i = 0; i < w * h; i++) {
        Color c = fb.getColor(i);
        data[idata] = (unsigned char)(c.r * 255);
        data[idata + 1] = (unsigned char)(c.g * 255);
        data[idata + 2] = (unsigned char)(c.b * 255);
        idata += 3;
    }
    OutputImage("rawdump", w, h, data);
    
    // tile
    fb.clear();
    int numTiles = fb.getNumTiles();
    for(int i = 0; i < numTiles; i++) {
        PPFloat ft = (PPFloat)i / (numTiles - 1.0);
        const FrameBuffer::Tile& tile = fb.getTile(i);
        
        for(int iy = tile.starty; iy < tile.endy; iy++) {
            PPFloat fy = (PPFloat)(iy - tile.starty) / (tile.height - 1.0);
            for(int ix = tile.startx; ix < tile.endx; ix++) {
                PPFloat fx = (PPFloat)(ix - tile.startx) / (tile.width - 1.0);
                int ib = tile.getPixelIndex(ix, iy);
                fb.accumulate(ib, Color(fx, fy, ft));
                //fb.accumulate(ix, iy, Color(fx, fy, ft));
            }
        }
    }
    
    // dump
    idata = 0;
    for(int iy = 0; iy < h; iy++) {
        for(int ix = 0; ix < w; ix++) {
            Color c = fb.getColor(ix, iy);
            data[idata] = (unsigned char)(c.r * 255);
            data[idata + 1] = (unsigned char)(c.g * 255);
            data[idata + 2] = (unsigned char)(c.b * 255);
            idata += 3;
        }
    }
    OutputImage("tiledump", w, h, data);
    
    delete [] data;
}
