#include "../include/extractMainArtist.h"
#include <vector>

std::string extractMainArtist(const std::string& artist) {
    if (artist.empty()) return "";

    std::vector<std::string> separators = { " feat ", " feat. ", " ft ",
        " ft. ", " vs ", " vs. ", " with ", ", ", " + ", " & "};
    size_t min_pos = std::string::npos;

    for (const auto& sep : separators) {
        size_t pos = artist.find(sep);
        if (pos != std::string::npos && (min_pos == std::string::npos || pos < min_pos)) {
            min_pos = pos;
        }
    }

    if (min_pos != std::string::npos) {
        std::string main = artist.substr(0, min_pos);

        size_t start = main.find_first_not_of(" \t");
        if (start == std::string::npos) return "";

        size_t end = main.find_last_not_of(" \t");
        return main.substr(start, end - start + 1);
    }

    return artist;
}