#include "../include/processDirectory.h"
#include "../include/getMp3Files.h"
#include "../include/constants.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/mpegfile.h>

namespace fs = std::filesystem;

bool processDirectory(const std::string& dir_path) {
    std::vector<fs::path> mp3_files = getMp3Files(dir_path);

    if (mp3_files.empty()) {
        std::cout << "No MP3 files found in directory: " << dir_path << std::endl;
        return false;
    }

    std::sort(mp3_files.begin(), mp3_files.end(), [](const std::string& a, const std::string& b) {
        TagLib::MPEG::File fileA(a.c_str());
        TagLib::MPEG::File fileB(b.c_str());
        return fileA.tag()->artist().to8Bit(true) < fileB.tag()->artist().to8Bit(true);
    });

    std::vector<std::string> artists_to_delete;
    size_t ArtistAudioCount = 1;

    TagLib::MPEG::File previous_file(mp3_files[0].c_str());
    std::string previous_artist = previous_file.tag()->artist().to8Bit(true);

    for (auto it = mp3_files.begin() + 1; it != mp3_files.end(); ++it) {
        TagLib::MPEG::File current_file(it->c_str());
        if (!current_file.isValid()) {
            std::cerr << "Error opening file: " << *it << std::endl;
            continue;
        }

        std::string current_artist = current_file.tag()->artist().to8Bit(true);

        if (current_artist == previous_artist) {
            ArtistAudioCount++;
        } else {
            if (ArtistAudioCount <= Constants::MIN_TRACKS_COUNT) {
                artists_to_delete.push_back(previous_artist);
            }
            previous_artist = current_artist;
            ArtistAudioCount = 1;
        }
    }

    if (ArtistAudioCount <= Constants::MIN_TRACKS_COUNT) {
        artists_to_delete.push_back(previous_artist);
    }

    if (!artists_to_delete.empty()) {
        size_t deleted_count = 0;

        sort(artists_to_delete.begin(), artists_to_delete.end());
        artists_to_delete.erase(unique(artists_to_delete.begin(), artists_to_delete.end()), 
                               artists_to_delete.end());

        std::cout << "Artists with less than " << Constants::MIN_TRACKS_COUNT << " tracks will be deleted:" << std::endl;
        for (const auto& artist : artists_to_delete) {
            std::cout << " - " << artist << std::endl;
        }

        for (const auto & file_path : mp3_files) {
            TagLib::MPEG::File file(file_path.c_str());
            if (!file.isValid()) continue;

            std::string current_artist = file.tag()->artist().to8Bit(true);

            if (find(artists_to_delete.begin(), artists_to_delete.end(), current_artist) != artists_to_delete.end()) {
                try {
                    if (fs::remove(file_path)) {
                        std::cout << "Deleted: " << file_path.filename() << std::endl;
                        deleted_count++;
                    }
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Error deleting " << file_path << ": " << e.what() << std::endl;
                }
            }
        }
    }
    std::cout << "No artists with less than " << Constants::MIN_TRACKS_COUNT << " tracks found" << std::endl;
    return false;
}