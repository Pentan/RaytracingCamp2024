#ifndef SPECTRENOTES_FRAMEBUFFER_H
#define SPECTRENOTES_FRAMEBUFFER_H

#include "types.h"

namespace Spectrenotes {
    
    class FrameBuffer {
        
    public:
        struct Pixel {
            Color accumulatedColor;
            int sampleCount;
            
            void clear() {
                accumulatedColor.set(0.0, 0.0, 0.0);
                sampleCount = 0;
            }
            
            void accumulate(const Color& c) {
                accumulatedColor += c;
                sampleCount += 1;
            }
            
            void setColor(const Color& c) {
                accumulatedColor = c;
                sampleCount = 1;
            }
            
            Color getColor() {
                return accumulatedColor / sampleCount;
            }
        };
        
        struct LayeredPixel {
            Pixel diffuse;
            Pixel specular;
            Pixel transmit;
            Pixel scatter;
            int totalCount;
        };
        
        struct RenderLayerPixel {
            LayeredPixel direct;
            LayeredPixel indirect;
        };
        
        struct Tile {
            int startx, endx;
            int starty, endy;
            int bufferStart;
            int width;
            int height;
            
            // tx:[startx, endx), ty:[starty, endy)
            int getPixelIndex(int tx, int ty) const {
                return bufferStart + (tx - startx) + (ty - starty) * width;
            }
        };
        
    public:
        FrameBuffer(int w, int h, int tsize);
        ~FrameBuffer();
        
        void clear();
        
        // buffer offset
        void accumulate(int i, const Color& col);
        void setColor(int i, const Color& col);
        Color getColor(int i) const;
        Pixel& getPixel(int i) const;
        
        // position
        void accumulate(int x, int y, const Color& col);
        void setColor(int x, int y, const Color& col);
        Color getColor(int x, int y) const;
        Pixel& getPixel(int x, int y) const;
        
        int getWidth() const { return width; }
        int getHeight() const { return height; }
        int getTileSize() const { return tileSize; }
        int getTileCols() const { return tileCols; }
        int getTileRows() const { return tileRows; }
        int getNumTiles() const { return tileCols * tileRows; }
        const Tile& getTile(int i) const { return tiles[i]; }
        
    private:
        Pixel *buffer;
        Tile *tiles;
        int width;
        int height;
        int tileSize;
        int tileRows;
        int tileCols;
        
        int positionToBufferIndex(int x, int y) const;
    };
    
    
}

#endif
