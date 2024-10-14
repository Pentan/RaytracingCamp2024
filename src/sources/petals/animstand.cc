#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <random>
#include <chrono>
#include <cmath>

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

#include "animstand.h"
#include "texture.h"
#include "camera.h"
#include "framebuffer.h"

using namespace Petals;

namespace {
    constexpr RTFloat kCelThickness = 0.000125; // [m]

    bool writeFrameBufferToFile(const FrameBuffer& fb, std::string savepath, RTFloat exportGamma = 2.2) {
        int w = fb.getWidth();
        int h = fb.getHeight();

        std::vector<unsigned char> rgb8buf;
        rgb8buf.resize(w * h * 3);

        auto encodeTo8byte = [](RTColorType c, double gamma) {
            c = std::max(0.0, std::min(1.0, c));
            c = pow(c, 1.0 / gamma);
            return static_cast<unsigned char>(std::max(0.0, std::min(255.0, c * 256.0)));
            };

        for (int iy = 0; iy < h; iy++) {
            for (int ix = 0; ix < w; ix++) {
                int ipxl = (ix + (h - iy - 1) * w) * 3;
                Color col = fb.getColor(ix, iy);
                rgb8buf[ipxl + 0] = encodeTo8byte(col.r, exportGamma);
                rgb8buf[ipxl + 1] = encodeTo8byte(col.g, exportGamma);
                rgb8buf[ipxl + 2] = encodeTo8byte(col.b, exportGamma);
            }
        }

        int saved = stbi_write_png(savepath.c_str(), w, h, 3, rgb8buf.data(), 0);
        return saved != 0;
    }
}

//================
bool Cel::loadFile(const std::string srcpath, const std::filesystem::path curdir) {
    std::filesystem::path fpath(srcpath);
    if (fpath.is_relative()) {
        fpath = curdir / fpath;
    }

    std::string pathstr = fpath.u8string();

    int x, y, c;
    stbi_uc* imgbuf = stbi_load(pathstr.c_str(), &x, &y, &c, 4);
    if (imgbuf == nullptr) {
        std::cerr << "stbi_load failed:" << pathstr << std::endl;
        return false;
    }

    texptr = std::make_shared<ImageTexture>(x, y);
    texptr->initWith8BPPImage(imgbuf, c, 2.2);
    stbi_image_free(imgbuf);

    // TODO
    texptr->setWrap(ImageTexture::WrapType::kClamp, ImageTexture::WrapType::kClamp);

    return true;
}

//================
//#define PETALS_WORK_SERIAL

bool AnimationStand::render() {
    std::random_device rnd_dev;
    std::mt19937 mt(rnd_dev());

    int currentFrame = 0;
    for (const auto& cutname : sequence) {
        const auto cutptr = cutList[cutname];
        const auto* cut = cutptr.get();

        for (int ifrm = 0; ifrm < cut->lastFrame; ifrm++) {
#ifdef PETALS_WORK_SERIAL
            // serial job
            RenderContexts cntx;
            cntx.cutptr = cutptr;
            cntx.cutFrameIndex = ifrm;
            cntx.serialFrameIndex = currentFrame;
            cntx.rng.setSeed(mt());

            std::cout << " start [" << cutname << "][" << ifrm << "] : " << currentFrame << std::endl;
            renderOneFrame(cntx);
#else
            // parallel job setup
            auto& rndrinfo = renderJobQueue.emplace();
            rndrinfo.cutName = cutname;
            rndrinfo.cutFrameIndex = ifrm;
            rndrinfo.serialFrameIndex = currentFrame;
#endif

            currentFrame += 1;
        }
    }

#ifndef PETALS_WORK_SERIAL
    // worker setup
    {
        int hwThreads = std::thread::hardware_concurrency();
        std::cout << "hardware_concurrency: " << hwThreads << std::endl;
        maxThreads = (maxThreads <= 0) ? hwThreads : maxThreads;
        std::cout << "maxThreads: " << maxThreads << std::endl;
    }
    
    workingCount = maxThreads;
    renderCntx.resize(maxThreads);
    workerPool.reserve(maxThreads);
    for (int i = 0; i < maxThreads; i++) {
        // init context
        auto& cntx = renderCntx[i];
        cntx.rng.setSeed(mt());
        // thread job
        auto& thrd = workerPool.emplace_back([&] {
            RenderInfo rndrinfo;
            while(true) {
                {
                    std::lock_guard<std::mutex> lock(renderJobQueueMutex);
                    if (renderJobQueue.empty()) {
                        break;
                    }
                    rndrinfo = renderJobQueue.front();
                    renderJobQueue.pop();
                }

                cntx.cutFrameIndex = rndrinfo.cutFrameIndex;
                cntx.serialFrameIndex = rndrinfo.serialFrameIndex;
                cntx.cutptr = this->cutList[rndrinfo.cutName];
                this->renderOneFrame(cntx);
            }
            workingCount -= 1;
        });
    }

    // wait to finish jobs
    std::cout << "time limit:" << limitSec << " [sec]" << std::endl;
    // FIXME
    constexpr int kWaitMilliSec = 1000;
    constexpr int kLogCount = 5;
    int loopCount = 0;
    while (workingCount > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(kWaitMilliSec));
        if (loopCount % kLogCount == 0) {
            std::cout << "working: " << workingCount << std::endl;
        }
        loopCount += 1;
        if (limitSec > 0.0) {
            if (loopCount * kWaitMilliSec / 1000.0 >= limitSec) {
                break;
            }
        }
    }
    // cleanup
    for (auto& thrd : workerPool) {
        thrd.join();
    }
#endif

    return true;
}

bool AnimationStand::renderOneFrame(RenderContexts& cntx) {
    // current cut
    const auto* cut = cntx.cutptr.get();

    // prepare framebuffer
    FrameBuffer framebuffer(outconf.width, outconf.height, 4096);
    framebuffer.clear();
    int fbw = framebuffer.getWidth();
    int fbh = framebuffer.getHeight();

    FrameBuffer tmpfb(outconf.width, outconf.height, 4096);

    RTFloat aspect = static_cast<RTFloat>(outconf.height) / outconf.width;
    Camera camera;
    camera.sensorWidth = outconf.filmWidth;
    camera.sensorHeight = outconf.filmHeight * aspect;
    camera.focusPlaneWidth = outconf.frameWidth;
    camera.focusPlaneHeight = outconf.frameHeight;
    camera.initWithType(Camera::CameraType::kFocusPlanePerspectiveCamera);

    StandLayout standLayout;

    size_t numshots = cut->shots.size();
    for (size_t ishot = 0; ishot < numshots; ishot++) {
        // current shot
        const auto* shot = cut->shots[ishot].get();

        RTFloat baseDistance = shot->camera.height;

        // camera setting
        {
            const auto& shotcam = shot->camera;
            camera.fNumber = shotcam.F;
            camera.focalLength = shotcam.focalLength;
            camera.focusDistance = baseDistance;

            // focus target plane
            if (!shotcam.focusPlane.empty()) {
                auto fndite = shot->stand.planeMap.find(shotcam.focusPlane);
                if (fndite == shot->stand.planeMap.end()) {
                    std::cerr << "focus target plane not found:" << shotcam.focusPlane << std::endl;
                    return false;
                }
                else {
                    const auto target = fndite->second;
                    camera.focusDistance -= target->height;
                }
            }
            camera.focusDistance += shotcam.focusShift;
        }

        // stand layout
        {
            standLayout.planes.clear();

            const auto& stand = shot->stand;
            for (const auto& pln : stand.planes) {
                // plane
                RTFloat plnBaseDist = baseDistance - pln.height;

                if (pln.itemName.empty()) {
                    // dummy cel
                    //auto& lyplane = standLayout.planes.emplace_back();
                    //lyplane.celptr = nullptr;
                    //lyplane.offset.set(0.0, 0.0, plnBaseDist);
                    continue;
                }

                const auto& psfnd = cut->planeSetups.find(pln.itemName);
                if (psfnd == cut->planeSetups.end()) {
                    std::cerr << "cut plane item not found:" << pln.itemName << std::endl;
                    return false;
                }

                const auto& pln_setup = psfnd->second;
                RTFloat plateRemain = pln_setup.size() - 1;
                for (const auto& pltptr : pln_setup) {
                    // for plate
                    const auto* pltobj = pltptr.get();
                    const auto tsfnd = cut->timesheet.find(pltobj->itemName);
                    if (tsfnd == cut->timesheet.end()) {
                        std::cerr << "cut plate item not found:" << pln.itemName << std::endl;
                        return false;
                    }

                    const auto& timesheet = tsfnd->second;
                    const auto& celname = (timesheet.size() > cntx.cutFrameIndex) ? timesheet[cntx.cutFrameIndex] : timesheet.back();

                    const auto bnkfnd = cut->bank.find(celname);
                    if (bnkfnd == cut->bank.end()) {
                        std::cerr << "timesheet item[" << cntx.cutFrameIndex << "] not found:" << celname << std::endl;
                        return false;
                    }

                    auto& lyplane = standLayout.planes.emplace_back();
                    lyplane.celptr = bnkfnd->second;
                    lyplane.offset.set(0.0, 0.0, 0.0);
                    lyplane.offset.z = plnBaseDist - plateRemain * kCelThickness;
                    plateRemain -= 1.0;
                }
            }
        }

        // render shot
        {
            auto& rng = cntx.rng;
            const auto& shotcam = shot->camera;
            const auto& rndconf = shotcam.render;
            tmpfb.clear();

            RGBColor topLitColor(0.0, 0.0, 0.0);
            if (shot->stand.toplight.enable) {
                topLitColor = shot->stand.toplight.color * shot->stand.toplight.power;
            }

            RGBColor backLitColor(0.0, 0.0, 0.0);
            if (shot->stand.backlight.enable) {
                backLitColor = shot->stand.backlight.color * shot->stand.backlight.power;
            }

            for (int iy = 0; iy < fbh; iy++) {
                for (int ix = 0; ix < fbw; ix++) {
                    //RTFloat tx = static_cast<RTFloat>(ix) / fbw;
                    //RTFloat ty = static_cast<RTFloat>(iy) / fbh;

                    //+++++ FIXME +++++
                    int sscol = 1;
                    int ssrow = 1;
                    if (rndconf.sampleStrategy == AnimCamera::SampleStrategy::kStratify) {
                        sscol = rndconf.sampleOption.stratify.cols;
                        ssrow = rndconf.sampleOption.stratify.rows;
                    }
                    //+++++

                    for(int isp = 0; isp < rndconf.sampleCount; isp++) {
                        for (int ssy = 0; ssy < ssrow; ssy++) {
                            for (int ssx = 0; ssx < sscol; ssx++) {
                                RTFloat stx = (ssx + rng.nextDoubleCO()) / sscol;
                                RTFloat sty = (ssy + rng.nextDoubleCO()) / ssrow;

                                // image origin is left top, world y up is positive
                                RTFloat sx = (ix + stx) / fbw * 2.0 - 1.0;
                                RTFloat sy = (iy + sty) / fbh * -2.0 + 1.0;

                                // camera is (0,0,0)
                                Ray ray = camera.getRay(sx, sy, &rng);

                                // TODO
                                // filter
                                //for(const auto& fltr : shotcam.filters)
                                //{
                                //    switch (fltr.type) {
                                //        case AnimCamera::FilterType::kSoft:
                                //            break;
                                //        case AnimCamera::FilterType::kCross:
                                //            break;
                                //        default:
                                //    }
                                //}

                                RGBColor tmpcolor(0.0, 0.0, 0.0);
                                RTFloat alpha = 1.0;
                                for (const auto& lypln : standLayout.planes) {
                                    RTFloat t = lypln.offset.z / ray.direction.z;
                                    Vector3 hit = ray.direction * t + ray.origin;
                                    const auto* celobj = lypln.celptr.get();

                                    RTFloat u = hit.x / (celobj->width * 0.001) + 0.5;
                                    RTFloat v = hit.y / (celobj->height * 0.001) + 0.5;

                                    const auto* celtex = celobj->texptr.get();
                                    auto texel = celtex->sample(u, v, true);
                                    tmpcolor = tmpcolor + texel.rgb * texel.a * alpha;
                                    alpha *= 1.0 - texel.a;
                                    if (alpha <= 0.0) break;
                                }

                                tmpcolor = RGBColor::mul(topLitColor, tmpcolor) + backLitColor * alpha;
                                tmpfb.accumulate(ix, iy, tmpcolor);
                            }
                        } // ss
                    } // sampleCount
                }
            } // iy
        } //

        // 
        for (int iy = 0; iy < fbh; iy++) {
            for (int ix = 0; ix < fbw; ix++) {
                // rendered image is 180 deg rotated
                auto tmpcolor = tmpfb.getColor(fbw - ix - 1, fbh - iy - 1);
                auto& pixel = framebuffer.getPixel(ix, iy);
                pixel.accumulatedColor += tmpcolor * shot->camera.exposure;
                pixel.sampleCount = 1;
            }
        }

        ////+++++
        //{
        //    const auto* cel = cut->bank.at("1").get();
        //    for (int iy = 0; iy < fbh; iy++) {
        //        for (int ix = 0; ix < fbw; ix++) {
        //            RTFloat tx = static_cast<RTFloat>(ix) / fbw;
        //            RTFloat ty = static_cast<RTFloat>(iy) / fbh;
        //            auto tmpcolor = cel->texptr->sample(tx, ty, true);
        //            framebuffer.setColor(ix, iy, tmpcolor.rgb);
        //        }
        //    }
        //}
        ////+++++

    } // shot

    // save frame
    std::stringstream ss;
    if (!outconf.directory.empty()) {
        ss << outconf.directory << "/";
    }
    if (!outconf.baseName.empty()) {
        ss << outconf.baseName;
    }
    ss << std::setfill('0') << std::right << std::setw(3);
    ss << cntx.serialFrameIndex << ".png";

    const auto savepath = ss.str();
    bool saveres = writeFrameBufferToFile(framebuffer, savepath);

    std::cout << savepath << " saved: " << saveres << std::endl;

    return saveres;
}
