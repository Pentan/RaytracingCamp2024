#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <random>

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

#include "animstand.h"
#include "texture.h"
#include "camera.h"
#include "framebuffer.h"

using namespace Spectrenotes;

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

    return true;
}

//================
bool AnimationStand::render() {
    //+++++
    // setup job worker
    //+++++
    std::random_device rnd_dev;
    std::mt19937 mt(rnd_dev());

    int currentFrame = 0;
    for (const auto& cutname : sequence) {
        const auto cutptr = cutList[cutname];
        const auto* cut = cutptr.get();

        for (int ifrm = 0; ifrm < cut->lastFrame; ifrm++) {
            RenderContexts cntx;
            cntx.cutptr = cutptr;
            cntx.cutFrameIndex = ifrm;
            cntx.serialFrameIndex = currentFrame;
            cntx.rng.setSeed(mt());

#if 1
            // serial job
            renderOneFrame(cntx);
#else
            // parallel job setup
#endif

            currentFrame += 1;
            break; //+++++
        }
    }

    //+++++
    // wait to finish jobs
    //+++++

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
            const auto& rndconf = shot->camera.render;
            tmpfb.clear();
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

                    for (int ssy = 0; ssy < ssrow; ssy++) {
                        for (int ssx = 0; ssx < sscol; ssx++) {
                            RTFloat stx = (ssx + rng.nextDoubleCO()) / sscol;
                            RTFloat sty = (ssy + rng.nextDoubleCO()) / ssrow;

                            RTFloat sx = (ix + stx) / fbw * 2.0 - 1.0;
                            RTFloat sy = (iy + sty) / fbh * 2.0 - 1.0;

                            // camera is (0,0,0)
                            Ray ray = camera.getRay(sx, sy, &rng);

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

                            tmpfb.accumulate(ix, iy, tmpcolor);
                        }
                    } // ss
                }
            } // iy
        } //

        // 
        for (int iy = 0; iy < fbh; iy++) {
            for (int ix = 0; ix < fbw; ix++) {
                auto tmpcolor = tmpfb.getColor(ix, iy);
                auto& pixel = framebuffer.getPixel(ix, iy);
                pixel.accumulatedColor += tmpcolor;
                pixel.sampleCount = 1;
            }
        }

        //+++++
        {
            const auto* cel = cut->bank.at("1").get();
            for (int iy = 0; iy < fbh; iy++) {
                for (int ix = 0; ix < fbw; ix++) {
                    RTFloat tx = static_cast<RTFloat>(ix) / fbw;
                    RTFloat ty = static_cast<RTFloat>(iy) / fbh;
                    auto tmpcolor = cel->texptr->sample(tx, ty, true);
                    framebuffer.setColor(ix, iy, tmpcolor.rgb);
                }
            }
        }
        //+++++

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
    return saveres;
}
