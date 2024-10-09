#ifndef SPECTRENOTES_ANIMSTAND_H
#define SPECTRENOTES_ANIMSTAND_H

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <thread>
#include <mutex>
#include <queue>

#include "types.h"
#include "random.h"

namespace Spectrenotes {

    class ImageTexture;

    //
    class AnimCamera {
    public:
        enum class RenderMode
        {
            kRGB,
            kSpectrum
        };
        enum class SampleStrategy
        {
            kRandom,
            kStratify,
            kSobol
        };
        struct RenderSetting {
            RenderMode mode;
            int sampleCount;
            SampleStrategy sampleStrategy;

            union {
                struct {
                    int rows;
                    int cols;
                } stratify;
            } sampleOption;
        };

        enum class FilterType
        {
            kSoft,
            kCross
        };
        struct Filter {
            FilterType type;
            RTFloat angle;

            union {
                struct {
                    RTFloat alha;
                } soft;

                struct {
                    RTFloat pitch;
                    RTFloat width;
                    int lines;
                } cross;
            } option;
        };

    public:
        AnimCamera() {}
        ~AnimCamera() {}

        RTFloat height;
        RTFloat exposure;
        RTFloat F;
        RTFloat focusShift;
        RTFloat focalLength;
        std::string focusPlane;
        std::vector<std::string> animation;
        std::vector<Filter> filters;
        RenderSetting render;
    };

    class CameraStand {
    public:
        struct LightSetting {
            bool enable;
            RTFloat power;
            RGBColor color;
            // Spectrum spectrum;

            LightSetting() :
                enable(false),
                power(0.0),
                color(1.0, 1.0, 1.0)
            {}
        };

        struct Plane {
            std::string id;
            std::string itemName;
            RTFloat height;
        };

    public:
        CameraStand() {}
        ~CameraStand() {}

        LightSetting toplight;
        LightSetting backlight;
        std::vector<Plane> planes;
        std::map<std::string, Plane*> planeMap;
    };

    class Shot {
    public:
        Shot() {}
        ~Shot() {}

        AnimCamera camera;
        CameraStand stand;
    };

    class Cel {
    public:
        // AJA layout specification
        // >> main frame size is 10 x 5.625 [inch] (254 x 142.875 [mm])
        Cel(const std::string& nm) : name(nm), width(254.0), height(142.875) {};
        ~Cel() {}

        bool loadFile(const std::string srcpath, const std::filesystem::path curdir);

        std::string name;
        std::shared_ptr<ImageTexture> texptr;
        RTFloat width;  // [mm]
        RTFloat height; // [mm]
    };

    class Plate {
    public:
        Plate() {};
        ~Plate() {}

        std::string itemName;
        std::string animName;
    };

    class Cut {

    public:
        Cut(const std::string& nm) : name(nm) {};
        ~Cut() {}

        std::string name;
        int lastFrame;
        std::map<std::string, std::shared_ptr<Cel>> bank;
        std::map<std::string, std::vector<std::string>> timesheet;
        std::vector<std::shared_ptr<Shot>> shots;
        std::map<std::string, std::vector<std::shared_ptr<Plate>>> planeSetups;

    };

    class StandLayout {
        struct LayoutPlane {
            std::shared_ptr<Cel> celptr;
            Vector3 offset;
        };

    public:
        StandLayout() {};
        ~StandLayout() {};

        std::vector<LayoutPlane> planes;
    };

    class AnimationStand {
    public:
        struct VersionInfo {
            int major;
            int minor;
        };

        struct OutputConfig {
            int width; // [pixel]
            int height;
            RTFloat fps;
            RTFloat filmWidth;  // [mm]
            RTFloat filmHeight;
            RTFloat frameWidth; // [m]
            RTFloat frameHeight;
            std::string directory;
            std::string baseName;
        };

    public:
        AnimationStand() {};
        ~AnimationStand() {};

        bool render();


    public:
        VersionInfo version;
        OutputConfig outconf;

        std::vector<std::string> sequence;
        std::map<std::string, std::shared_ptr<Cut>> cutList;

        int maxThreads;
        RTFloat limitSec;

    private:
        struct RenderContexts {
            std::shared_ptr<Cut> cutptr;
            Random rng;
            int cutFrameIndex;
            int serialFrameIndex;
        };
        bool renderOneFrame(RenderContexts& cntx);

        struct RenderInfo {
            std::string cutName;
            int cutFrameIndex;
            int serialFrameIndex;
        };
        std::vector<std::thread> workerPool;
        std::queue<RenderInfo> renderJobQueue;
        std::mutex renderJobQueueMutex;
        std::atomic<int> workingCount;

        std::vector<RenderContexts> renderCntx;
    };

}

#endif
