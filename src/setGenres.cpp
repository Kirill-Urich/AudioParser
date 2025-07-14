#include "../include/setGenres.h"
#include "../include/getMp3Files.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/mpegfile.h>

bool setGenres(const std::string& dir_path) {
    if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
        std::cerr << "Directory does not exist or is not accessible: " << dir_path << std::endl;
        return false;
    }

    std::ifstream infile;
    infile.open("../ArtistGenres.txt");
    if (!infile.is_open()) {
        std::cerr << "File ArtistGenres.txt can't be opened!";
        return false;
    }

    std::unordered_map<std::string, std::string> artistGenres;

    std::string line;
    while (getline(infile, line)) {
        if (line.empty()) continue;

        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        size_t pos = line.find(" - ");
        if (pos != std::string::npos) {
            std::string artist = line.substr(0, pos);
            std::string genre = line.substr(pos + 3);

            genre.erase(0, genre.find_first_not_of(" \t"));
            genre.erase(genre.find_last_not_of(" \t") + 1);

            if (artistGenres.count(artist)) {
                std::cerr << "Warning: Duplicate artist found - " << artist << std::endl;
            }
            
            artistGenres[artist] = genre;
        }
    }
    infile.close();
    
    std::vector<fs::path> mp3_files = getMp3Files(dir_path);
    if (mp3_files.empty()) {
        std::cout << "No MP3 files found in directory: " << dir_path << std::endl;
        return false;
    }

    for (const auto & mp3_path : mp3_files) {
        TagLib::MPEG::File file(mp3_path.c_str());
        if (!file.isValid()) {
            std::cerr << "Error opening file: " << mp3_path << std::endl;
            continue;
        }
        
        TagLib::Tag *tag = file.tag();
        if (!tag || tag->artist().isEmpty()) {
            std::cerr << "No tags found in file: " << mp3_path << std::endl;
            continue;
        }

        std::string artist = tag->artist().to8Bit(true);
        auto it = artistGenres.find(artist);

        if (it == artistGenres.end()) {
            std::cerr << "Genre not found for artist: " << artist << " (" << mp3_path << ")" << std::endl;
            continue;
        }

        tag->setGenre(TagLib::String(it->second, TagLib::String::UTF8));

        if (!file.save()) {
            std::cerr << "Failed to save changes to file: " << mp3_path << std::endl;
        }
    }

    return true;
}