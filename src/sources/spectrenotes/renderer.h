#ifndef SPECTRENOTES_RENDERER_H
#define SPECTRENOTES_RENDERER_H

#include <vector>
#include <memory>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include "random.h"
#include "ray.h"

namespace Spectrenotes {
    
    class Scene;
    class Config;
    class FrameBuffer;
    class PostProcessor;
    
    class Renderer {
    private:
        // config
        int samplesPerPixel;
        int pixelSubSamples;
        
        int minDepth;
        int maxDepth;
        RTFloat minRussianRouletteCutOff;
        
        int renderFrames;
        int startFrame;
        double fps;
        double exposureSec;
        int exposureSlice;
        std::string saveNameBase;
        std::string saveExt;
        
        double limitSecPerFrame;
        double limitSecMargin;
        double progressIntervalSec;
        
        // contexts
        struct Context {
            Random random;
            FrameBuffer* framebuffer;
            PostProcessor* postprocessor;
            RTTimeType exposureTimeRate;
        };
        
        std::vector<Context> renderContexts;
        int numMaxJobs;
        
        struct RenderResult {
            Color radiance;
            
            int depth;
            
            Color directRadiance;
            Color indirectRadiance;
            
            //
            Vector3 firstNormal;
            Color firstAlbedo;
            int firstHitMeshId;
            int firstHitClusterId;
            int firstHitPrimitiveId;
            
            Vector3 firstDiffuseNormal;
            Color firstDiffuseAlbedo;
            
            void clear();
        };
        
        enum CommandType {
            kNoop,
            kRender,
            kPostprocess,
            kSaveFile,
            kNumCommandType
        };
        
        struct JobCommand {
            CommandType type;
            int jobid;
            
            union {
                struct {
                    int frameId;
                    int tileInfoIndex;
                    int samples;
                    int subSamples;
                } render;
                struct {
                    PostProcessor* processor;
                    int jobIndex;
                } postprocess;
                struct {
                    PostProcessor* processor;
                } save;
            };
        };
        
        enum WorkerStatus {
            kStart,
            kWaiting,
            kProcessing,
            kDone,
            kStopped,
            kNumWorkerStatus
        };
        
        struct WorkerInfo {
            CommandType commandType;
            WorkerStatus status;
            int infoValue0;
            int infoValue1;
        };

        struct TileInfo {
            RTTimeType processTime;
            int tileIndex;
        };
        
    public:
        std::vector<std::unique_ptr<FrameBuffer> > framebuffers;
        std::vector<std::unique_ptr<PostProcessor> > postprocessors;
        Scene *scene;
        
    public:
        Renderer(const Config& config, Scene* scn);
        ~Renderer();
        
        void render();
        void pathtrace(const Ray& iray, const Scene* scn, Context* cntx, RenderResult *result);
        
        static void startWorker(int workerid, Renderer* rndr);
        void wokerMain(int workerid);
        void processCommand(int workerid, JobCommand cmd);
        
        void renderJob(int workerid, JobCommand cmd);
        void postprocessJob(int workerid, JobCommand cmd);
        void saveFileJob(int workerid, JobCommand cmd);
    private:
        
        size_t frameBufferIndex;
        // FrameBuffer* currentFramebuffer;
        std::vector<std::thread> workerPool;
        std::vector<WorkerInfo> workerInfos;
        // std::mutex workerInfoMutex;
        std::atomic<int> processingWorkerCount;
        std::queue<JobCommand> commandQueue;
        std::queue<JobCommand> interruptQueue;
        std::mutex commandQueueMutex;
        std::condition_variable workerCondition;
        std::condition_variable managerCondition;
        bool stopWorkers;
        int renderingFrameId;

        std::vector<TileInfo> tileInfos;
        
        void pushRenderCommands(FrameBuffer* fb, int frameID, int spp, int ss);
        void renderOneFrame(FrameBuffer* fb, PostProcessor* pp, RTTimeType opentime, RTTimeType closetime, int frameId);
        void postProcessAndSave(FrameBuffer* fb, PostProcessor* pp, int frameid);
        void waitAllCommands();
        void waitAllAndLog();
        void waitRenderUntil(FrameBuffer* fb, int frameId, double startTime, double timeLimit);
        double checkPrintProcessLog(double logedTime);
        void processAllCommands();
        void setupWorkers();
        void cleanupWorkers();
    };
}

#endif
