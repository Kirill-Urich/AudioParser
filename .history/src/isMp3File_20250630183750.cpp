#include "../include/isMp3File.h"
#include <algorithm>

bool isMp3File(const std::string& filename) {
    size_t dotPos = filename.find_last_of(".");
    if (dotPos != std::string::npos) {
        std::string ext = filename.substr(dotPos + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext == "mp3";
    }
    return false;
}