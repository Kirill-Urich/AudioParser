#include "../include/getMp3Files.h"
#include "../include/processFile.h"
#include "../include/constants.h"
#include "../include/processDirectory.h"
#include "../include/setGenres.h"
#include "../include/setAlbums.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory>" << std::endl;
        return 1;
    }

    fs::path directory(argv[1]);
    std::vector<fs::path> mp3Files = getMp3Files(directory);

    if (mp3Files.empty()) {
        std::cout << "No MP3 files found in directory: " << directory << std::endl;
        return 0;
    }

    std::cout << "Found " << mp3Files.size() << " MP3 files in " << directory << std::endl;
    std::cout << "Processing artist tags..." << std::endl;

    int changed = 0;

    for (const auto& filepath : mp3Files) {
        if (processFile(filepath)) {
            changed++;
        }
    }

    std::cout << "\nProcessing complete!" << std::endl;
    std::cout << "Total processed: " << mp3Files.size() << std::endl;
    std::cout << "Files changed: " << changed << std::endl;
    std::cout << "Files unchanged: " << mp3Files.size() - changed << std::endl;


    std::cout << "Scanning for artists with less than " << Constants::MIN_TRACKS_COUNT << " tracks..." << std::endl;

    if (processDirectory(directory)) {
        std::cout << "Cleanup completed." << std::endl;
    } else {
        std::cout << "No files were deleted." << std::endl;
    }

    std::cout << "Scanning for genres in mp3 files in " << directory << std::endl;
    if (setGenres(directory)) {
        std::cout << "setGenres is completed." << std::endl;
    } else {
        std::cout << "setGenres if failed." << std::endl;
    }

    std::cout << "Scanning for albums in mp3 files in " << directory << std::endl;
    if (setAlbums(directory)) {
        std::cout << "setAlbums is completed." << std::endl;
    } else {
        std::cout << "setAlbums if failed." << std::endl;
    }

    return 0;
}