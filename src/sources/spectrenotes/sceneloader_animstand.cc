#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <vector>
#include <memory>
#include <map>
#include <filesystem>

#include <nlohmann/json.hpp>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

#include "sceneloader.h"
#include "texture.h"
#include "camera.h"
#include "keyframesampler.h"

using namespace Spectrenotes;

namespace {
    //
    template <typename T> T GetContaintValue(const nlohmann::json& json, std::string key, T defaultValue, bool* outerr=nullptr) {
        if (json.contains(key)) {
            if (outerr != nullptr) *outerr = true;
            return json[key].get<T>();
        }
        if (outerr != nullptr) *outerr = false;
        return defaultValue;
    }

    template <typename T> bool CheckContaintValue(const nlohmann::json& json, std::string key, T *outval ) {
        if (json.contains(key)) {
            if(outval) *outval = json[key].get<T>();
            return true;
        }
        return false;
    }

    //
    class Cel {
    public:
        // AJA layout specification
        // >> ÉÅÉCÉìÉtÉåÅ[ÉÄÇÃÉTÉCÉYÇÕÅAÇPÇOÉCÉìÉ`Å~ÇTÅDÇUÇQÇTÉCÉìÉ` (ÇQÇTÇSmmÅ~ÇPÇSÇQÅDÇWÇVÇTmm)
        Cel(const std::string& nm) : name(nm), width(254.0), height(142.875) {};
        ~Cel() {}

        bool loadFile(const std::string srcpath, const std::filesystem::path curdir) {
            return true;
        }

        std::string name;
        std::shared_ptr<ImageTexture> texptr;
        RTFloat width;
        RTFloat height;
    };

    class Cut {
    public:
        Cut(const std::string& nm): name(nm) {};
        ~Cut() {}

        std::string name;
        int lastFrame;
        std::map<std::string, std::shared_ptr<Cel>> bank;
    };

    class AnimationStand {
    public:
        struct VersionInfo {
            int major;
            int minor;
        };

        struct OutputConfig {
            int width;
            int height;
            RTFloat fps;
            RTFloat filmSize;
            std::string directory;
            std::string baseName;
        };

    public:
        AnimationStand() {};
        ~AnimationStand() {};

    public:
        VersionInfo version;
        OutputConfig outconf;

        std::vector<std::string> sequence;
        std::map<std::string, std::shared_ptr<Cut>> cutList;
    };

    //
    bool CheckMagic(const nlohmann::json& json, AnimationStand::VersionInfo *verinfo) {
        if (!json.contains("animstand")) return false;
        const auto& magic = json["animstand"];
        if (verinfo) {
            verinfo->major = GetContaintValue<int>(magic, "major_version", 0);
            verinfo->minor = GetContaintValue<int>(magic, "minor_version", 0);
        }
        return true;
    }

    //
    bool ParsePlaneSetup(const nlohmann::json& json, std::vector<std::filesystem::path>& pathstack, Cut* cutobj) {
        if (!json.contains("plane_setup")) {
            std::cerr << "plane_setup not found" << std::endl;
            return false;
        }
        // array

        return true;
    }

    bool ParseAnimation(const nlohmann::json& json, std::vector<std::filesystem::path>& pathstack, Cut* cutobj) {
        if (!json.contains("animation")) {
            std::cerr << "animation not found" << std::endl;
            return false;
        }
        // obj

        return true;
    }

    bool ParseShot(const nlohmann::json& json, std::vector<std::filesystem::path>& pathstack, Cut* cutobj) {
        if (!json.contains("shot")) {
            std::cerr << "shot not found" << std::endl;
            return false;
        }
        // array

        return true;
    }

    bool ParseTimeSheet(const nlohmann::json& json, std::vector<std::filesystem::path>& pathstack, Cut* cutobj) {
        if (!json.contains("timesheet")) {
            std::cerr << "timesheet not found" << std::endl;
            return false;
        }
        // obj

        return true;
    }

    bool ParseBank(const nlohmann::json& json, std::vector<std::filesystem::path>& pathstack, Cut* cutobj) {
        if (!json.contains("bank")) {
            std::cerr << "bank not found" << std::endl;
            return false;
        }
        const auto& bank = json["bank"];
        if (!bank.is_array()) {
            std::cerr << "bank is not array" << std::endl;
            return false;
        }

        for (const auto& img : bank) {
            bool noerr = true;
            std::string name;
            std::string src;
            noerr |= CheckContaintValue(img, "name", &name);
            noerr |= CheckContaintValue(img, "source", &src);
            if (!noerr) {
                return false;
            }

            auto celptr = std::make_shared<Cel>(name);
            auto* celobj = celptr.get();

            noerr |= celobj->loadFile(src, pathstack.back());
            if (!noerr) {
                return false;
            }

            std::string size;
            if (CheckContaintValue(img, "size", &size)) {
                if (size.back() == '%') {
                    auto scale = static_cast<RTFloat>(std::stod(size));
                    celobj->width *= scale;
                    celobj->height *= scale;
                } else if(size.find(':') != std::string::npos) {
                    std::size_t tmppos;
                    celobj->width = static_cast<RTFloat>(std::stod(size, &tmppos));
                    size = size.substr(tmppos + 1);
                    celobj->height = static_cast<RTFloat>(std::stod(size, &tmppos));
                }
            }

            cutobj->bank.emplace(celptr->name, celptr);
        }

        return true;
    }

    bool ParseCutContent(const nlohmann::json& json, std::vector<std::filesystem::path>& pathstack, Cut* cutobj) {
        bool noerr = true;
        noerr &= CheckContaintValue(json, "last_frame", &cutobj->lastFrame);
        noerr &= ParseBank(json, pathstack, cutobj);
        noerr &= ParseTimeSheet(json, pathstack, cutobj);
        noerr &= ParseAnimation(json, pathstack, cutobj);
        noerr &= ParsePlaneSetup(json, pathstack, cutobj);
        noerr &= ParseShot(json, pathstack, cutobj);
        return noerr;
    }

    bool ParseCut(const nlohmann::json& json, std::vector<std::filesystem::path>& pathstack, AnimationStand* animstand) {
        if (!json.contains("name")) {
            std::cerr << "cut requires name" << std::endl;
            return false;
        }

        auto cutName = json["name"].get<std::string>();
        auto cutptr = std::make_shared<Cut>(cutName);
        auto* cutobj = cutptr.get();
        bool noerr = true;
        auto* srcjson = &json;
        if (json.contains("source")) {
            const auto srcpath = json["source"].get<std::string>();
            std::filesystem::path cutpath(srcpath);
            nlohmann::json srcjson;

            if (cutpath.is_absolute()) {
                pathstack.push_back(cutpath.parent_path());
            } else {
                auto cdirpath = pathstack.back();
                pathstack.push_back(cdirpath);
                cutpath = cdirpath / cutpath;
            }

            std::ifstream fs(cutpath);
            if (!fs.is_open()) {
                std::cerr << "cut file couldn't open:" << cutpath << std::endl;
                return false;
            }
            fs >> srcjson;
            noerr = ParseCutContent(srcjson, pathstack, cutobj);
            pathstack.pop_back();
        } else if(json.contains("cut")) {
            noerr = ParseCutContent(json["cut"], pathstack, cutobj);
        }

        if (noerr) {
            animstand->cutList.emplace(cutName, cutptr);
        }
        
        return noerr;
    }

    bool ParseCutList(const nlohmann::json& json, std::vector<std::filesystem::path>& pathstack, AnimationStand* animstand) {
        if (!json.contains("cut_list")) {
            std::cerr << "cut_list not found" << std::endl;
            return false;
        }
        const auto& cutlist = json["cut_list"];

        if (!cutlist.is_array()) return false;
        for (const auto& cut : cutlist) {
            if (!ParseCut(cut, pathstack, animstand)) {
                return false;
            }
        }
        return true;
    }

    bool ParseMovieOutput(const nlohmann::json& json, std::vector<std::filesystem::path>& pathstack, AnimationStand* animstand) {
        if (!json.contains("output")) {
            std::cerr << "output not found" << std::endl;
            return false;
        }
        const auto& output = json["output"];
        animstand->outconf.width = GetContaintValue<int>(output, "width", 1920);
        animstand->outconf.height = GetContaintValue<int>(output, "height", 1080);
        animstand->outconf.fps = GetContaintValue<RTFloat>(output, "fps", 24.0);
        animstand->outconf.filmSize = GetContaintValue<RTFloat>(output, "film_size", 35.0);
        animstand->outconf.directory = GetContaintValue<std::string>(output, "directory", std::string("output"));
        animstand->outconf.baseName = GetContaintValue<std::string>(output, "base_name", std::string("frame"));
        return true;
    }

    bool ParseMovieSequence(const nlohmann::json& json, std::vector<std::filesystem::path>& pathstack, AnimationStand* animstand) {
        if (!json.contains("sequence")) {
            std::cerr << "sequence not found" << std::endl;
            return false;
        }
        const auto& sequence = json["sequence"];
        if (!sequence.is_array()) return false;
        for (const auto& cutid : sequence) {
            animstand->sequence.push_back(cutid.get<std::string>());
        }
        return true;
    }

    bool ParseMovie(const nlohmann::json& json, std::vector<std::filesystem::path>& pathstack, AnimationStand* animstand) {
        if (!json.contains("movie")) {
            std::cerr << "movie not found" << std::endl;
            return false;
        }
        const auto& movie = json["movie"];
        bool noerr = true;
        noerr &= ParseMovieOutput(movie, pathstack, animstand);
        noerr &= ParseMovieSequence(movie, pathstack, animstand);
        return noerr;
    }
}

AssetLibrary* SceneLoader::loadAnimStand(std::string filepath) {
    std::filesystem::path fpath(filepath);

    std::ifstream fs(fpath);
    if (!fs.is_open()) {
        std::cerr << "scene file couldn't open:" << filepath << std::endl;
        return nullptr;
    }

    nlohmann::json jsonRoot;
    fs >> jsonRoot;

    if (!jsonRoot.is_object()) {
        std::cerr << "scene file json format error:" << filepath << std::endl;
        return nullptr;
    }

    AnimationStand::VersionInfo verinfo;
    if (!CheckMagic(jsonRoot, &verinfo)) {
        std::cerr << "scene file has no animstand magic:" << filepath << std::endl;
        return nullptr;
    }

    std::vector<std::filesystem::path> pathstack;
    if (fpath.has_parent_path()) {
        pathstack.push_back(fpath.parent_path());
    }
    else {
        pathstack.push_back(std::filesystem::path());
    }

    auto* animstand = new AnimationStand();
    animstand->version = verinfo;

    bool noerr = true;
    noerr &= ParseCutList(jsonRoot, pathstack, animstand);
    noerr &= ParseMovie(jsonRoot, pathstack, animstand);

    if (!noerr) {
        delete animstand;
        animstand = nullptr;
    }

    //return animstand;
    return nullptr;
}



