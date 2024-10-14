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

#include "animstand.h"
#include "sceneloader.h"
#include "texture.h"
#include "camera.h"
#include "keyframesampler.h"

using namespace Petals;

namespace {
    //
    template <typename T> T GetContaintValue(const nlohmann::json& json, std::string key, T defaultValue, bool* outerr = nullptr) {
        if (json.contains(key)) {
            if (outerr != nullptr) *outerr = true;
            return json[key].get<T>();
        }
        if (outerr != nullptr) *outerr = false;
        return defaultValue;
    }

    template <typename T> bool CheckContaintValue(const nlohmann::json& json, std::string key, T* outval) {
        if (json.contains(key)) {
            if (outval) *outval = json[key].get<T>();
            return true;
        }
        return false;
    }

    //
    bool CheckMagic(const nlohmann::json& json, AnimationStand::VersionInfo* verinfo) {
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
        const auto& planesetups = json["plane_setup"];
        if (!planesetups.is_array()) {
            std::cerr << "plane_setup is not array" << std::endl;
            return false;
        }

        for (const auto& setup : planesetups) {
            if (!setup.contains("name") || !setup.contains("plates")) {
                std::cerr << "plane_setup data error" << std::endl;
                return false;
            }
            const auto& name = setup["name"].get<std::string>();
            const auto& plates = setup["plates"];
            if (!plates.is_array()) {
                std::cerr << "plates data error" << std::endl;
                return false;
            }

            auto& empres = cutobj->planeSetups.emplace(name, std::vector<std::shared_ptr<Plate>>());
            auto& pltarray = empres.first->second;
            for (const auto& plt : plates) {
                auto pltptr = std::make_shared<Plate>();
                auto* pltobj = pltptr.get();
                pltobj->itemName = GetContaintValue<std::string>(plt, "item", std::string());
                pltobj->animName = GetContaintValue<std::string>(plt, "animation", std::string());
                pltarray.push_back(pltptr);
            }
        }

        return true;
    }

    bool ParseAnimation(const nlohmann::json& json, std::vector<std::filesystem::path>& pathstack, Cut* cutobj) {
        if (!json.contains("animation")) {
            std::cerr << "animation not found" << std::endl;
            return false;
        }
        const auto& animation = json["animation"];
        if (!animation.is_object()) {
            std::cerr << "animation is not object" << std::endl;
            return false;
        }

        for (const auto& animkv : animation.items()) {
            const auto& animkey = animkv.key();
            const auto& animval = animkv.value();
            if (!animval.is_array()) {
                std::cerr << "animation key frame format error" << std::endl;
                return false;
            }

            for (const auto& keyframe : animval) {
                for (const auto& kfkv : keyframe.items()) {
                    // FIXME
                    const auto& kfkey = kfkv.key();
                    const auto& kfval = kfkv.value();

                    if (kfkey.compare("frame") == 0) {
                        int frame = kfval.get<int>();
                    }
                    else if (kfkey.compare("focus") == 0) {
                        const auto& target = kfval.get<std::string>();
                    }
                    else if (kfkey.compare("ease_in") == 0) {
                        const auto& ease = kfval.get<std::string>();
                    }
                    else if (kfkey.compare("ease_out") == 0) {
                        const auto& ease = kfval.get<std::string>();
                    }
                    else {
                        RTFloat numval = kfval.get<RTFloat>();
                    }
                }
            }
        }

        return true;
    }

    bool ParseShotContent(const nlohmann::json& json, std::vector<std::filesystem::path>& pathstack, Cut* cutobj) {
        auto shotptr = std::make_shared<Shot>();
        auto* shotobj = shotptr.get();

        // camera
        {
            const auto& camjson = json["camera"];
            auto& camera = shotobj->camera;

            camera.height = GetContaintValue<RTFloat>(camjson, "height", 1.0);
            camera.focusShift = GetContaintValue<RTFloat>(camjson, "focus_shift", 0.0);
            camera.exposure = GetContaintValue<RTFloat>(camjson, "exposure", 1.0);
            camera.F = GetContaintValue<RTFloat>(camjson, "F", 2.0);
            camera.focalLength = GetContaintValue<RTFloat>(camjson, "focal_length", 100.0);
            camera.focusPlane = GetContaintValue<std::string>(camjson, "focus", "");

            if (camjson.contains("filter")) {
                const auto& filters = camjson["filter"];
                for (const auto& filter : filters) {
                    if (!filter.contains("type")) {
                        std::cerr << "filter require type" << std::endl;
                        return false;
                    }
                    const auto& type = filter["type"].get<std::string>();

                    auto& filterobj = camera.filters.emplace_back();
                    if (type.compare("soft") == 0) {
                        filterobj.type = AnimCamera::FilterType::kSoft;
                        filterobj.option.soft.alha = GetContaintValue(filter, "roughness", 0.2);
                        filterobj.option.soft.alha *= filterobj.option.soft.alha;
                    }
                    else if (type.compare("cross") == 0) {
                        filterobj.type = AnimCamera::FilterType::kCross;
                        filterobj.option.cross.pitch = GetContaintValue<RTFloat>(filter, "pitch", 5.0);
                        filterobj.option.cross.width = GetContaintValue<RTFloat>(filter, "width", 0.2);
                        filterobj.option.cross.lines = GetContaintValue<int>(filter, "lines", 4);
                    }
                    else {
                        std::cerr << "unknown filter type:" << type << std::endl;
                    }
                }
            }

            if (camjson.contains("animation")) {
                const auto& anims = camjson["animation"];
                for (const auto& animname : anims) {
                    const auto& name = animname.get<std::string>();
                    camera.animation.push_back(name);
                }
            }

            if (camjson.contains("render")) {
                const auto& rndrjson = camjson["render"];

                const auto& mode = GetContaintValue<std::string>(rndrjson, "mode", "rgb");
                if (mode.compare("rgb") == 0) {
                    camera.render.mode = AnimCamera::RenderMode::kRGB;
                }
                else if (mode.compare("spectrum") == 0) {
                    camera.render.mode = AnimCamera::RenderMode::kSpectrum;
                }

                camera.render.sampleCount = GetContaintValue<int>(rndrjson, "sample_count", 1);

                const auto& strategy = GetContaintValue<std::string>(rndrjson, "sample_strategy", "stratify");
                if (strategy.find("stratify") != std::string::npos) {
                    camera.render.sampleStrategy = AnimCamera::SampleStrategy::kStratify;
                    camera.render.sampleOption.stratify.cols = 1;
                    camera.render.sampleOption.stratify.rows = 1;

                    auto fndpos = strategy.find(':');
                    if (fndpos != std::string::npos) {
                        auto tmp = strategy.substr(fndpos + 1);
                        int cols = std::stoi(tmp, &fndpos);
                        int rows = cols;
                        if (fndpos + 1 < tmp.length()) {
                            tmp = tmp.substr(fndpos + 1);
                            rows = std::stoi(tmp, &fndpos);
                        }

                        if (cols > 0) {
                            camera.render.sampleOption.stratify.cols = cols;
                            camera.render.sampleOption.stratify.rows = rows;
                        }
                    }
                }
                else if (strategy.compare("sobol") == 0) {
                    camera.render.sampleStrategy = AnimCamera::SampleStrategy::kSobol;
                }
                else {
                    camera.render.sampleStrategy = AnimCamera::SampleStrategy::kRandom;
                }
            }
        }

        // stand
        {
            const auto& standjson = json["stand"];
            auto& stand = shotobj->stand;

            if (standjson.contains("lights")) {
                const auto& lights = standjson["lights"];
                for (int i = 0; i < 2; i++) {
                    const nlohmann::json* plitjson;
                    CameraStand::LightSetting* standlit;
                    if (i == 0) {
                        if (!lights.contains("top")) continue;
                        plitjson = &lights["top"];
                        standlit = &stand.toplight;
                    }
                    else {
                        if (!lights.contains("back")) continue;
                        plitjson = &lights["back"];
                        standlit = &stand.backlight;
                    }

                    const auto& litjson = *plitjson;
                    standlit->enable = GetContaintValue<bool>(litjson, "enable", false);
                    standlit->power = GetContaintValue<RTFloat>(litjson, "power", 0.0);
                    if (litjson.contains("color")) {
                        const auto& coljson = litjson["color"];
                        if (!coljson.is_object()) {
                            std::cerr << "color require {\"r\":R, \"g\":G, \"b\":B}" << std::endl;
                            return false;
                        }
                        standlit->color.r = GetContaintValue<RTFloat>(coljson, "r", 1.0);
                        standlit->color.g = GetContaintValue<RTFloat>(coljson, "g", 1.0);
                        standlit->color.b = GetContaintValue<RTFloat>(coljson, "b", 1.0);
                    }
                }
            }

            if (standjson.contains("planes")) {
                const auto& planesjson = standjson["planes"];
                if (!planesjson.is_array()) {
                    std::cerr << "planes is not array" << std::endl;
                    return false;
                }

                auto planecount = planesjson.size();
                stand.planes.reserve(planecount);

                for (const auto& plnjson : planesjson) {
                    auto& curplane = stand.planes.emplace_back();
                    curplane.id = GetContaintValue<std::string>(plnjson, "id", std::string(""));
                    curplane.itemName = GetContaintValue<std::string>(plnjson, "item", std::string(""));
                    curplane.height = GetContaintValue<RTFloat>(plnjson, "height", 0.0);
                    stand.planeMap.emplace(curplane.id, &curplane);
                }
            }
        }

        cutobj->shots.push_back(shotptr);
        return true;
    }

    bool ParseShot(const nlohmann::json& json, std::vector<std::filesystem::path>& pathstack, Cut* cutobj) {
        if (!json.contains("shot")) {
            std::cerr << "shot not found" << std::endl;
            return false;
        }
        const auto& shot = json["shot"];
        if (!shot.is_array()) {
            std::cerr << "shot is not array" << std::endl;
            return false;
        }

        for (const auto& shotitem : shot) {
            bool noerr = ParseShotContent(shotitem, pathstack, cutobj);
            if (!noerr) {
                return false;
            }
        }

        return true;
    }

    bool ParseTimeSheet(const nlohmann::json& json, std::vector<std::filesystem::path>& pathstack, Cut* cutobj) {
        if (!json.contains("timesheet")) {
            std::cerr << "timesheet not found" << std::endl;
            return false;
        }
        const auto& timesheet = json["timesheet"];
        if (!timesheet.is_object()) {
            std::cerr << "timesheet is not object" << std::endl;
            return false;
        }

        for (const auto& item : timesheet.items()) {
            const auto& key = item.key();
            const auto& cels = item.value();
            if (!cels.is_array()) {
                std::cerr << "timesheet format error. require key:[array]" << std::endl;
                return false;
            }
            auto& empres = cutobj->timesheet.emplace(key, std::vector<std::string>());
            if (!empres.second) {
                std::cerr << "timesheet allocation failed. less memory?" << std::endl;
                return false;
            }
            auto& sheet = empres.first->second;
            for (const auto celname : cels) {
                sheet.push_back(celname);
            }
        }

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
                }
                else if (size.find(':') != std::string::npos) {
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
            }
            else {
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
        }
        else if (json.contains("cut")) {
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
        const auto& outputjson = json["output"];
        animstand->outconf.width = GetContaintValue<int>(outputjson, "width", 1920);
        animstand->outconf.height = GetContaintValue<int>(outputjson, "height", 1080);
        animstand->outconf.fps = GetContaintValue<RTFloat>(outputjson, "fps", 24.0);
        animstand->outconf.filmWidth = 24.0;
        animstand->outconf.filmHeight = 18.0;
        if (outputjson.contains("film_size")) {
            const auto& film_size = outputjson["film_size"];
            if (film_size.is_object()) {
                animstand->outconf.filmWidth = GetContaintValue<RTFloat>(film_size, "width", 24.0);
                animstand->outconf.filmHeight = GetContaintValue<RTFloat>(film_size, "height", 18.0);
            }
            else {
                const auto& tmpstr = film_size.get<std::string>();
                if (tmpstr.compare("35mm") == 0 || tmpstr.compare("movie 35mm") == 0) {
                    animstand->outconf.filmWidth = 24.0;
                    animstand->outconf.filmHeight = 18.0;
                }
                else if (tmpstr.compare("still 35mm") == 0 || tmpstr.compare("photo 35mm") == 0) {
                    animstand->outconf.filmWidth = 36.0;
                    animstand->outconf.filmHeight = 24.0;
                }
                else if (tmpstr.compare("16mm") == 0) {
                    animstand->outconf.filmWidth = 10.26;
                    animstand->outconf.filmHeight = 7.49;
                }
                else {
                    std::cerr << "unknown film_size:" << tmpstr << std::endl;
                    return false;
                }
            }
        }

        animstand->outconf.frameWidth = 254.0;
        animstand->outconf.frameHeight = 142.875;
        if (outputjson.contains("frame_size")) {
            const auto& frame_size = outputjson["frame_size"];
            if (!frame_size.is_object()) {
                std::cerr << "frame_size is not object. requires {\"width\":x, \"height\":y}" << std::endl;
                return false;
            }
            animstand->outconf.frameWidth = GetContaintValue<RTFloat>(frame_size, "width", 254.0);
            animstand->outconf.frameHeight = GetContaintValue<RTFloat>(frame_size, "height", 142.875);
        }
        animstand->outconf.frameWidth /= 1000.0; // [mm->m]
        animstand->outconf.frameHeight /= 1000.0;

        animstand->outconf.directory = GetContaintValue<std::string>(outputjson, "directory", std::string(""));
        animstand->outconf.baseName = GetContaintValue<std::string>(outputjson, "base_name", std::string(""));
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

AnimationStand* SceneLoader::loadAnimStand(std::string filepath) {
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

    return animstand;
}



