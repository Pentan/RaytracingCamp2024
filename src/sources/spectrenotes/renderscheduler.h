#ifndef SPECTRENOTES_RENDER_SCHEDULER_H
#define SPECTRENOTES_RENDER_SCHEDULER_H

namespace Spectrenotes {
    
    class Renderer;
    
    class RenderScheduler {
    public:
        enum RenderState {
            kNone,
            kStarted,
            kRendering,
            kStopping,
            kStoped,
            kFinished
        };
        
    public:
        RenderScheduler();
        
        void render(const Renderer* renderer);
        void renderAsync(const Renderer* renderer);
        void terminate();
        RenderState getRenderState();
    };
}

#endif
