#include "framebuffer.h"
using namespace Spectrenotes;

FrameBuffer::FrameBuffer(int w, int h, int tsize):
    width(w),
    height(h),
    tileSize(tsize)
{
    buffer = new Pixel[width * height];
    
    tileCols = (width + tsize -1) / tsize;
    tileRows = (height + tsize -1) / tsize;
    
    tiles = new Tile[tileCols * tileRows];
    int bufoffset = 0;
    for(int ty = 0; ty < tileRows; ty++) {
        for(int tx = 0; tx < tileCols; tx++) {
            int ti = tx + ty * tileCols;
            Tile *tile = &tiles[ti];
            
            tile->startx = tx * tileSize;
            tile->endx = std::min(width, tile->startx + tileSize);
            tile->starty = ty * tileSize;
            tile->endy = std::min(height, tile->starty + tileSize);
            
            tile->width = tile->endx - tile->startx;
            tile->height = tile->endy - tile->starty;
            
            tile->bufferStart = bufoffset;
            bufoffset += tile->width * tile->height;
        }
    }
    
    clear();
}

FrameBuffer::~FrameBuffer() {
    delete [] buffer;
    delete [] tiles;
}

void FrameBuffer::clear() {
    int buflen = width * height;
    for(int i = 0; i < buflen; i++) {
        buffer[i].clear();
    }
}

void FrameBuffer::accumulate(int i, const Color& col) {
    buffer[i].accumulate(col);
}

void FrameBuffer::accumulate(int x, int y, const Color& col) {
    int i = positionToBufferIndex(x, y);
    accumulate(i, col);
}

void FrameBuffer::setColor(int i, const Color& col) {
    buffer[i].setColor(col);
}

void FrameBuffer::setColor(int x, int y, const Color& col) {
    int i = positionToBufferIndex(x, y);
    setColor(i, col);
}

Color FrameBuffer::getColor(int  i) const {
    return buffer[i].getColor();
}

FrameBuffer::Pixel& FrameBuffer::getPixel(int  i) const {
    return buffer[i];
}

Color FrameBuffer::getColor(int x, int y) const {
    int i = positionToBufferIndex(x, y);
    return getColor(i);
}

FrameBuffer::Pixel& FrameBuffer::getPixel(int x, int y) const {
    int i = positionToBufferIndex(x, y);
    return buffer[i];
}

int FrameBuffer::positionToBufferIndex(int x, int y) const {
    int tilex = x / tileSize;
    int tiley = y / tileSize;
    
    int tileIndex = tilex + tiley * tileCols;
    Tile &tile = tiles[tileIndex];
    
    int subx = x - tilex * tileSize;
    int suby = y - tiley * tileSize;
    int subindex = subx + suby * tile.width;
    
    return tile.bufferStart + subindex;
}
