#include "renderscheduler.h"
#include "renderer.h"


using namespace PinkyPi;

RenderScheduler::RenderScheduler() {
}


void RenderScheduler::render(const Renderer* renderer) {
    
}

void RenderScheduler::renderAsync(const Renderer* renderer) {
    
}

void RenderScheduler::terminate() {
    
}

RenderScheduler::RenderState RenderScheduler::getRenderState() {
    return RenderState::kNone;
}
