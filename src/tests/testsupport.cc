#include <sstream>
#include <cstdio>
#include "testsupport.h"

using namespace PinkyPi;

#ifndef PINKYPI_NO_STD_FILESYSTEM
#include <filesystem>
namespace {
bool CheckExistsOrCreateDir(std::string dirpath) {
    const std::filesystem::path path(dirpath);
    if(!std::filesystem::exists(path)) {
        if(!std::filesystem::create_directory(path)) {
            return false;
        }
    }
    return true;
}
}
#else
#include <dirent.h>
#include <sys/stat.h>
namespace {
bool CheckExistsOrCreateDir(std::string dirpath) {
    DIR *dir = opendir(dirpath.c_str());
    if(dir == NULL) {
        if(mkdir(dirpath.c_str(), ACCESSPERMS) != 0) {
            return false;
        }
    } else {
        closedir(dir);
    }
    return true;
}
}
#endif

void PinkyPi::CheckTestOutputDir(std::string dirpath) {
    if(!CheckExistsOrCreateDir(PINKYPI_TEST_OUTPUT_DIR)) {
        return;
    }
    
    std::stringstream ss;
    ss << PINKYPI_TEST_OUTPUT_DIR << "/" << dirpath;
    std::string fullpath = ss.str();
    if(!CheckExistsOrCreateDir(fullpath)) {
        return;
    }
}
