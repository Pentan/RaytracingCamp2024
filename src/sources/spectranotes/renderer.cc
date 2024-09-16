#include <ctime>
#include <iostream>
#include <sstream>
#include <ios>
#include <iomanip>
#include "renderer.h"
#include "config.h"
#include "scene.h"
#include "camera.h"
#include "framebuffer.h"
#include "random.h"
#include "ray.h"
#include "postprocessor.h"
#include "node.h"
#include "texture.h"
#include "material.h"
#include "mesh.h"
#include "bvh.h"
#include "assetlibrary.h"

#define USE_NEE 1

using namespace PinkyPi;

namespace {
    template<class T> void shuffleVector(std::vector<T>& v, Random& rng) {
        size_t vsize = v.size();
        for(size_t i = 0; i < vsize - 1; i++) {
            size_t si = i + static_cast<size_t>(rng.nextDoubleCO() * (vsize - i));
            std::swap(v[i], v[si]);
        }
    }

    bool isValidIntersection(const Vector3& d, const SurfaceInfo& surf) {
        PPFloat ds = Vector3::dot(d, surf.shadingNormal);
        PPFloat gs = Vector3::dot(d, surf.geometryNormal);
        return ds * gs > 0.0;
    }
}

namespace TimeUtils {
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
    double getTimeInSeconds() {
        return timeGetTime() / 1000.0;
    }
#else
#include <sys/time.h>
    double getTimeInSeconds() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec + (double)tv.tv_usec * 1e-6;
    }
#endif

double markedTimeInSec = -1.0;

void setElapsedTimeMarker() {
    markedTimeInSec = getTimeInSeconds();
}

double getElapsedTimeInSeconds() {
    double ct = getTimeInSeconds();
    if(markedTimeInSec < 0.0) {
        markedTimeInSec = ct;
        return 0.0;
    }
    return ct - markedTimeInSec;
}
}

Renderer::Renderer(const Config& config, Scene* scn):
    scene(scn),
    frameBufferIndex(0)
{
    int numbuffers = config.framebufferStockCount;
    framebuffers.reserve(numbuffers);
    postprocessors.reserve(numbuffers);
    for(int i = 0; i < numbuffers; i++) {
        auto fb = new FrameBuffer(config.width, config.height, config.tileSize);
        framebuffers.push_back(std::unique_ptr<FrameBuffer>(fb));
        auto pp = new PostProcessor();
        postprocessors.push_back(std::unique_ptr<PostProcessor>(pp));
    }

    int numTiles = framebuffers[0].get()->getNumTiles();
    tileInfos.resize(numTiles);
    for(int i = 0; i < numTiles; i++) {
        tileInfos[i].processTime = 0.0;
        tileInfos[i].tileIndex = i;
    }
    
    samplesPerPixel = config.samplesPerPixel;
    pixelSubSamples = config.pixelSubSamples;
    
    minDepth = config.minDepth;
    maxDepth = config.maxDepth;
    minRussianRouletteCutOff = config.minRussianRouletteCutOff;
    
    renderFrames = config.frames;
    startFrame = config.startFrame;
    fps = config.framesPerSecond;
    exposureSec = config.exposureSecond;
    exposureSlice = config.exposureSlice;
    
    numMaxJobs = config.maxThreads;
    
    limitSecPerFrame = config.limitSec / config.frames;
    limitSecMargin = config.limitMargin;
    progressIntervalSec = config.progressIntervalSec;
    
    std::string saveDir = config.outputDir;
    if(saveDir.length() > 0) {
        if(saveDir[saveDir.length() - 1] != '/') {
            saveDir += "/";
        }
        saveNameBase = saveDir + config.outputName;
    } else {
        saveNameBase = config.outputName;
    }
    saveExt = config.outputExt;
}

Renderer::~Renderer() {
    cleanupWorkers();
}


void Renderer::pushRenderCommands(FrameBuffer* fb, int frameID, int spp, int ss) {
    int numTiles = fb->getNumTiles();
    {
        std::unique_lock<std::mutex> lock(commandQueueMutex);
        for (int i = 0; i < numTiles; i++) {
            JobCommand cmd;
            cmd.type = CommandType::kRender;
            cmd.render.tileInfoIndex = i;
            cmd.render.samples = spp;
            cmd.render.subSamples = ss;
            cmd.render.frameId = frameID;
            commandQueue.push(cmd);
        }
    }
    workerCondition.notify_all();
}

void Renderer::renderOneFrame(FrameBuffer* fb, PostProcessor* pp, PPTimeType opentime, PPTimeType closetime, int frameId) {
    // scene setup
    std::cout << "  scene [" << opentime << "," << closetime << "] setup (" << TimeUtils::getElapsedTimeInSeconds() << ")" << std::endl;
    scene->seekTime(opentime, closetime, exposureSlice, 0);
    
    // init contexts
    unsigned long seedbase = static_cast<unsigned long>(time(NULL)); // FIXME
    for(int i = 0; i < numMaxJobs; i++) {
        Context& cntx = renderContexts[i];
        cntx.random.setSeed(seedbase + i * 123456789);
        cntx.framebuffer = fb;
        cntx.postprocessor = pp;
    }

    // setup render jobs
    std::cout << "  submit render commands (" << TimeUtils::getElapsedTimeInSeconds() << ")" << std::endl;
    pushRenderCommands(fb, frameId, samplesPerPixel, pixelSubSamples);
}

void Renderer::postProcessAndSave(FrameBuffer* fb, PostProcessor* pp, int frameid) {
    std::stringstream ss;
    ss << saveNameBase;
    ss << std::setfill('0') << std::right << std::setw(4);
    ss << frameid << "." << saveExt;
    auto savepath = ss.str();
    
    int numjobs = pp->init(fb, savepath, 4096, 2.2, frameid);
    {
        std::unique_lock<std::mutex> lock(commandQueueMutex);
        for(int i = 0; i < numjobs; i++) {
            JobCommand cmd;
            cmd.type = CommandType::kPostprocess;
            cmd.postprocess.processor = pp;
            cmd.postprocess.jobIndex = i;
            commandQueue.push(cmd);
        }
    }
    workerCondition.notify_all();
}

void Renderer::render() {
    TimeUtils::setElapsedTimeMarker();

    if(numMaxJobs == 0) {
        numMaxJobs = std::thread::hardware_concurrency();
        std::cout << "maxJobs as hardware_concurrency: " << numMaxJobs << std::endl;
    }
    renderContexts.resize(numMaxJobs);
    
    if(numMaxJobs > 1) {
        setupWorkers();
    }
    
    for(int i = 0; i < renderFrames; i++) {
        double frameStartTime = TimeUtils::getTimeInSeconds();
        int frameNumber = i + startFrame + 1; // from 1 to N
        renderingFrameId = frameNumber;

        std::cout << "<" << i+1 << "/" << renderFrames << "> frame[" << frameNumber <<  "] start ("  << TimeUtils::getElapsedTimeInSeconds() << ")" << std::endl;
        
        auto fb = framebuffers[frameBufferIndex].get();
        fb->clear();
        auto pp = postprocessors[frameBufferIndex].get();
        
        PPTimeType t = (frameNumber - 1) / static_cast<PPTimeType>(fps); // from 0 to N-1
        renderOneFrame(fb, pp, t, t + exposureSec, frameNumber);
        
        // wait
        if(numMaxJobs <= 1) {
            // serial exection
            processAllCommands();
        } else {
            // concurrent
            if(limitSecPerFrame > 0.0) {
                waitRenderUntil(fb, frameNumber, frameStartTime, limitSecPerFrame);
            } else {
                waitAllCommands();
            }
        }
        
        // post process and save
        postProcessAndSave(fb, pp, frameNumber);
        if(numMaxJobs <= 1) {
            processAllCommands();
        }
        
        // next
        frameBufferIndex = (frameBufferIndex + 1) % framebuffers.size();
    }
    
    if(numMaxJobs > 1) {
        waitAllCommands();
    }
    cleanupWorkers();
}

void Renderer::pathtrace(const Ray& iray, const Scene* scn, Context* cntx, RenderResult *result)
{
    Random& rng = cntx->random;
    result->clear();
    
    Color throughput(1.0, 1.0, 1.0);
    Color radiance(0.0, 0.0, 0.0);
    bool isHitDiffuse = false;
    
    std::vector<Vector3> uvbuf;
    uvbuf.reserve(4);
    SurfaceInfo surfinfo;
    Material::EvalLog materiallog;
    
    Ray ray = iray;
    Ray nextray;
    int depth = 0;
    bool isloop = true;
    while(isloop) {
        SceneIntersection intersect;
        IntersectionDetail detail;
        PPFloat hitt = scene->intersection(ray, kRayOffset, kFarAway, cntx->exposureTimeRate, &intersect);
        if(hitt <= 0.0) {
            // background
            //radiance = Color::mul(throughput, Color(1.0, 1.0, 1.0));
            //break;
#ifdef USE_NEE
            if (!isHitDiffuse || depth == 0) {
                auto texel = scene->backgroundTexture->sampleEquirectangular(ray.direction, false);
                radiance += Color::mul(throughput, texel.rgb);
            }
#else
            auto texel = scene->backgroundTexture->sampleEquirectangular(ray.direction, false);
            radiance += Color::mul(throughput, texel.rgb);
#endif
            break;
        }
        
        const MeshIntersection& meshisect = intersect.meshIntersect;
        scene->computeIntersectionDetail(ray, hitt, cntx->exposureTimeRate, intersect, &detail);
        auto* hitmaterial = scene->assetLib->getMaterial(detail.materialId);
        auto* hitmesh = scene->assetLib->getMesh(meshisect.meshId);
        
        if(detail.uvCount <= 1) {
            surfinfo.uv0 = &detail.texcoord0;
        } else {
            uvbuf.clear();
            Vector3& bc = detail.barycentricCoord;
            for(int iuv = 0; iuv < detail.uvCount; iuv++) {
                auto* uva = detail.vertexAttributes[0].uv0 + iuv;
                auto* uvb = detail.vertexAttributes[1].uv0 + iuv;
                auto* uvc = detail.vertexAttributes[2].uv0 + iuv;
                Vector3 tmpuv = *uva * bc.x + *uvb + bc.y + *uvc * bc.z;
                uvbuf.push_back(tmpuv);
            }
            surfinfo.uv0 = uvbuf.data();
        }
        
        // fill surface info
        surfinfo.position = ray.pointAt(hitt);
        surfinfo.geometryNormal = detail.geometryNormal;
        surfinfo.shadingNormal = hitmaterial->evaluateNormal(surfinfo.uv0, detail.shadingNormal, detail.shadingTangent);
        
        // emissiv
        auto hitemit = hitmaterial->evaluateEmissive(surfinfo.uv0);
        radiance += Color::mul(throughput, hitemit);

        // throughput
        auto hitthp = hitmaterial->evaluateThroughput(ray, &nextray, surfinfo, rng, &materiallog);
        if(materiallog.bxdfType == Material::kEmission) {
            break;
        }

#ifdef USE_NEE
        if(materiallog.bxdfType == Material::kDiffuse) {
            // sample bg
            auto smpl = Sampler::sampleCosineWeightedHemisphere(surfinfo.shadingNormal, rng);
            if (isValidIntersection(smpl.v, surfinfo)) {
                Ray shdwray(surfinfo.position, smpl.v);
                PPFloat shdwt = scene->intersection(shdwray, kRayOffset, kFarAway, cntx->exposureTimeRate, nullptr);
                if (shdwt < 0.0) {
                    Material::EvalLog shadowlog;
                    PPFloat fbxdf = hitmaterial->evaluateBXDF(ray, shdwray, materiallog.selectedBxdfId, surfinfo, &shadowlog);
                    auto texel = scene->backgroundTexture->sampleEquirectangular(shdwray.direction, false);
                    PPFloat lmb = std::max(0.0, Vector3::dot(shdwray.direction, surfinfo.shadingNormal));
                    Color col = Color::mul(materiallog.filterColor, texel.rgb);
                    radiance += Color::mul(throughput, col) * lmb * fbxdf / (shadowlog.bxdfPdf * smpl.pdf);
                }
            }

            // sample light
            if (!isHitDiffuse) {
                // all light
                for (auto lite = scene->lights.begin(); lite != scene->lights.end(); ++lite) {

                }
            } else {
                // one light
            }
        }
#endif

        //
        if(!isValidIntersection(ray.direction * -1.0, surfinfo)) {
            hitthp.set(0.0, 0.0, 0.0);
        }

        // first diffuse
        if (!isHitDiffuse && materiallog.bxdfType == Material::kDiffuse) {
            result->firstDiffuseAlbedo = materiallog.filterColor;
            result->firstDiffuseNormal = surfinfo.shadingNormal;
            isHitDiffuse = true;
        }

        // first hit
        if (depth == 0) {
            result->firstAlbedo = materiallog.filterColor;
            result->firstNormal = surfinfo.shadingNormal;
            result->firstHitMeshId = intersect.meshIntersect.meshId;
            result->firstHitClusterId = intersect.meshIntersect.clusterId;
            result->firstHitPrimitiveId = intersect.meshIntersect.triangleId;
        }

        // lambert
        PPFloat ndotd = Vector3::dot(nextray.direction, surfinfo.shadingNormal);
        if(ndotd < 0.0) {
            ndotd = 0.0;
        }
        throughput = Vector3::mul(hitthp, throughput) * std::abs(ndotd) / materiallog.pdf;
        
        //+++++
        //radiance.set(0.2, 0.2, 1.0);
        //radiance = detail.geometryNormal * 0.5 + 0.5;
        //radiance = detail.texcoord0;
        //break;

        //PPFloat dfs = Vector3::dot(detail.geometryNormal, Vector3(0.577, 0.577, 0.577)) * 0.5 + 0.5;
        //radiance = material->baseColorFactor * dfs;

        //radiance = hitmaterial->baseColorFactor * (hitt - std::floor(hitt));
        //radiance = throughput;
        //+++++
        
        if(depth > minDepth) {
            //PPFloat cutoff = throughput.getMaxComponent();
            PPFloat cutoff = (throughput.x + throughput.y + throughput.z) / 3.0;
            PPFloat q = std::max(minRussianRouletteCutOff, 1.0 - cutoff);
            if(rng.nextDoubleCO() < q){
                break;
            }
            throughput = throughput / (1.0 - q);
        }

        if(depth > maxDepth) {
            break;
        }
        
        ray = nextray;
        depth += 1;
        
        //break; //+++++
    }
    
    result->radiance = radiance;
    result->depth = depth;
}

void Renderer::renderJob(int workerid, JobCommand cmd) {
    // expired
    if (cmd.render.frameId != renderingFrameId) { return; }
    
    auto* workerinfo = &workerInfos[workerid];

    Context *cntx = &renderContexts[workerid];
    Random& rng = cntx->random;
    TileInfo& tileinfo = tileInfos[cmd.render.tileInfoIndex];
    int tileIndex = tileinfo.tileIndex;

    const FrameBuffer::Tile& tile = cntx->framebuffer->getTile(tileIndex);
    int spp = cmd.render.samples;
    int subsp = cmd.render.subSamples;

    PPFloat subPixelSize = 1.0 / subsp;
    Node* cameraNode = scene->cameras[0];
    Camera *camera = cameraNode->content.camera;
    
    RenderResult result;
    double starttime = TimeUtils::getTimeInSeconds();

    int numspi = subsp * subsp;
    std::vector<int> subPixelIndex(numspi);
    for (int i = 0; i < numspi; i++) {
        subPixelIndex[i] = i;
    }
    
    workerinfo->infoValue0 = tileIndex;
    
    for(int iy = tile.starty; iy < tile.endy; iy++) {
        for(int ix = tile.startx; ix < tile.endx; ix++) {
            PPFloat px = PPFloat(ix);
            PPFloat py = PPFloat(iy);
            int pixelId = tile.getPixelIndex(ix, iy);
            FrameBuffer::Pixel& pixel = cntx->framebuffer->getPixel(pixelId);
            
            workerinfo->infoValue1 = pixelId;
            
            for(int ips = 0; ips < spp; ips++) {
                if (cmd.render.frameId != renderingFrameId) { return; }

                int spi = ips % numspi;
                if (spi == 0) {
                    shuffleVector(subPixelIndex, rng);
                }
                PPFloat ssx = PPFloat(subPixelIndex[spi] % subsp) * subPixelSize;
                PPFloat ssy = PPFloat(subPixelIndex[spi] / subsp) * subPixelSize;

                PPFloat sx = px + ssx + rng.nextDoubleCO() * subPixelSize;
                PPFloat sy = py + ssy + rng.nextDoubleCO() * subPixelSize;

                sx = (sx / cntx->framebuffer->getWidth()) * 2.0 - 1.0;
                sy = (sy / cntx->framebuffer->getHeight()) * 2.0 - 1.0;

                Ray ray = camera->getRay(sx, sy, &rng);

                cntx->exposureTimeRate = rng.nextDoubleCO();
                Matrix4 camgm = cameraNode->computeGlobalMatrix(cntx->exposureTimeRate);

                ray = ray.transformed(camgm);
                pathtrace(ray, scene, cntx, &result);

                pixel.accumulate(result.radiance);
                
                //+++++
                //pixel.accumulate(result.firstNormal * 0.5 + 0.5);
                //break;
                //+++++
            }
        }
    }

    tileinfo.processTime = TimeUtils::getTimeInSeconds() - starttime;
}

void Renderer::postprocessJob(int workerid, JobCommand cmd) {
    auto pp = cmd.postprocess.processor;
    int remain = pp->process(cmd.postprocess.jobIndex);
    
    if(remain <= 1) {
        //{
        //    std::unique_lock<std::mutex> lock(commandQueueMutex);
        //    JobCommand cmd;
        //    cmd.type = CommandType::kSaveFile;
        //    cmd.save.processor = pp;
        //    interruptQueue.push(cmd);
        //}
        //workerCondition.notify_all();

        JobCommand cmd;
        cmd.type = CommandType::kSaveFile;
        cmd.save.processor = pp;
        saveFileJob(workerid, cmd);
    }
}

void Renderer::saveFileJob(int workerid, JobCommand cmd) {
    auto pp = cmd.save.processor;
    bool saved = pp->writeToFile(false);
    std::cout << "  frame [" << pp->frameId << "]";
    std::cout << " " << pp->savePath << (saved ? " saved." : " save failed.");
    std::cout << " (" << TimeUtils::getElapsedTimeInSeconds() << ")" << std::endl;
}

void Renderer::startWorker(int workerid, Renderer* rndr) {
    rndr->wokerMain(workerid);
}

void Renderer::wokerMain(int workerid) {
    auto* info = &workerInfos[workerid];
    
    auto changeInfoState = [this](WorkerInfo* info, WorkerStatus s) {
        // std::unique_lock<std::mutex> lock(workerInfoMutex);
        info->status = s;
        info->infoValue0 = 0;
        info->infoValue1 = 0;
    };

    while(true) {
        JobCommand cmd;
        // info->status = WorkerStatus::kWaiting;
        changeInfoState(info, WorkerStatus::kWaiting);
        info->commandType = CommandType::kNoop;
        {
            std::unique_lock<std::mutex> lock(commandQueueMutex);
            workerCondition.wait(lock, [this]{
                return !commandQueue.empty() || !interruptQueue.empty() || stopWorkers;
            });
            
            if(!stopWorkers) {
                processingWorkerCount.fetch_add(1);
                if(!interruptQueue.empty()) {
                    cmd = interruptQueue.front();
                    interruptQueue.pop();
                } else {
                    cmd = commandQueue.front();
                    commandQueue.pop();
                }
            }
        }
        
        if(stopWorkers) break;
        
        //info->status = WorkerStatus::kProcessing;
        changeInfoState(info, WorkerStatus::kProcessing);
        info->commandType = cmd.type;
        processCommand(workerid, cmd);

        //info->status = WorkerStatus::kDone;
        changeInfoState(info, WorkerStatus::kDone);
        processingWorkerCount.fetch_sub(1);

        managerCondition.notify_all();
    }
    
    info->status = WorkerStatus::kStopped;
    info->commandType = CommandType::kNoop;
}

void Renderer::processCommand(int workerid, JobCommand cmd) {
    switch (cmd.type) {
        case kRender:
            renderJob(workerid, cmd);
            break;
        case kPostprocess:
            postprocessJob(workerid, cmd);
            break;
        case kSaveFile:
            saveFileJob(workerid, cmd);
            break;
        default:
            break;
    }
}

void Renderer::waitAllCommands() {
    if (progressIntervalSec < 0.0) {
        std::unique_lock<std::mutex> lock(commandQueueMutex);
        managerCondition.wait(lock, [this] {
            //bool isempty = commandQueue.empty() && interruptQueue.empty();
            //bool isdone = true;
            //{
            //    std::unique_lock<std::mutex> lock(workerInfoMutex);
            //    for (auto ite = workerInfos.begin(); ite != workerInfos.end(); ++ite) {
            //        if (ite->status == WorkerStatus::kProcessing) {
            //            isdone = false;
            //        }
            //    }
            //}
            //return isempty && isdone;
            return commandQueue.empty() && interruptQueue.empty() && (processingWorkerCount.load() == 0);
            });
    } else {
        waitAllAndLog();
    }
}

void Renderer::waitAllAndLog() {
    // wait and log output
    const long kMinSleep = 100;
    long sleepMilliSec;
    if (progressIntervalSec < 0.0) {
        sleepMilliSec = kMinSleep;
    } else {
        sleepMilliSec = std::min(kMinSleep, static_cast<long>(progressIntervalSec * 1000.0));
    }
    bool waiting = true;
    double logedtime = 0.0;
    while (waiting) {
        // sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepMilliSec));
        
        // check
        bool isprocessing = false;
        for(auto ite = workerInfos.begin(); ite != workerInfos.end(); ++ite) {
            if(ite->status == WorkerStatus::kProcessing) {
                isprocessing = true;
                break;
            }
        }
        waiting = isprocessing || !commandQueue.empty() || !interruptQueue.empty();
        
        // log
        logedtime = checkPrintProcessLog(logedtime);
    }
}

void Renderer::waitRenderUntil(FrameBuffer* fb, int frameId, double startTime, double timeLimit) {
    const long kSleepMilliSec = 10;
    bool waiting = true;
    double logedtime = 0.0;
    int prevspp = samplesPerPixel;
    double sequenceStart = TimeUtils::getTimeInSeconds();
    while (waiting) {
        // sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(kSleepMilliSec));

        // timeout
        double elapsedTime = TimeUtils::getTimeInSeconds() - startTime;
        if (timeLimit - elapsedTime < limitSecMargin) {
            waiting = false;
            break;
        }

        // log
        logedtime = checkPrintProcessLog(logedtime);

        // command check
        if(commandQueue.empty() && interruptQueue.empty() && (processingWorkerCount.load() == 0)) {
            // refill
            std::sort(tileInfos.begin(), tileInfos.end(), [](TileInfo& a, TileInfo& b) {
                return a.processTime > b.processTime;
            });

            double curTime = TimeUtils::getTimeInSeconds();
            double sequenceTime = curTime - sequenceStart;
            double timeRemain = timeLimit - (curTime - startTime);
            
            int nextspp = static_cast<int>(prevspp * timeRemain / sequenceTime);
            if (timeRemain < sequenceTime) {
                nextspp = nextspp * 2 / 3;
            }
            nextspp = std::max(1, std::min(nextspp, prevspp));

            //std::cout << "  sequenceTime:" << sequenceTime << ",timeRemain:" << timeRemain << ",timeLimit:" << timeLimit << "\n";
            std::cout << "  refill render commands. spp:" << nextspp << " (" << TimeUtils::getElapsedTimeInSeconds() << ")" << std::endl;
            pushRenderCommands(fb, frameId, nextspp, pixelSubSamples);
            
            sequenceStart = curTime;
            prevspp = nextspp;
        }
    }
}

double Renderer::checkPrintProcessLog(double logedTime) {
    double curtime = TimeUtils::getElapsedTimeInSeconds();
    if (progressIntervalSec > 0.0 && curtime - logedTime > progressIntervalSec) {
        // print
        static const char cmdtbl[CommandType::kNumCommandType] = {
            'n', 'R', 'P', 'S'
        };
        static const char stattbl[WorkerStatus::kNumWorkerStatus] = {
            '$', '-', '#', '*'
        };
        std::stringstream ss;
        ss << "[" << numMaxJobs << "]:";
        for (auto ite = workerInfos.begin(); ite != workerInfos.end(); ++ite) {
            auto info = *ite;
            ss << " " << stattbl[info.status] << cmdtbl[info.commandType];
            ss << ":" << info.infoValue0 << "," << info.infoValue1;
        }
        std::cout << ss.str() << std::endl;

        logedTime = curtime;
    }
    return logedTime;
}

void Renderer::processAllCommands() {
    // for serial exection
    while(!interruptQueue.empty() || !commandQueue.empty()) {
        if(!interruptQueue.empty()) {
            auto cmd = interruptQueue.front();
            interruptQueue.pop();
            processCommand(0, cmd);
        } else {
            auto cmd = commandQueue.front();
            commandQueue.pop();
            processCommand(0, cmd);
        }
    }
}

void Renderer::setupWorkers() {
    if(numMaxJobs <= 1) return;
    processingWorkerCount = 0;
    workerInfos.resize(numMaxJobs);
    stopWorkers = false;
    workerPool.reserve(numMaxJobs);
    for(int i = 0; i < numMaxJobs; i++) {
        workerInfos[i].status = WorkerStatus::kStart;
        workerInfos[i].commandType = CommandType::kNoop;
        workerPool.emplace_back(startWorker, i, this);
    }
}

void Renderer::cleanupWorkers() {
    if(workerPool.size() == 0) {
        return;
    }
    
    // stop threads
    {
        std::unique_lock<std::mutex> lock(commandQueueMutex);
        stopWorkers = true;
    }
    workerCondition.notify_all();
    
    for(size_t i = 0; i < workerPool.size(); i++) {
        workerPool[i].join();
    }
    workerPool.clear();
}

void Renderer::RenderResult::clear() {
    radiance.set(0.0, 0.0, 0.0);
    depth = 0;
    
    directRadiance.set(0.0, 0.0, 0.0);
    indirectRadiance.set(0.0, 0.0, 0.0);
    
    firstNormal.set(0.0, 0.0, 0.0);
    firstAlbedo.set(0.0, 0.0, 0.0);
    
    firstHitMeshId = -1;
    firstHitClusterId = -1;
    firstHitPrimitiveId = -1;
    
    firstDiffuseNormal.set(0.0, 0.0, 0.0);
    firstDiffuseAlbedo.set(0.0, 0.0, 0.0);
}
