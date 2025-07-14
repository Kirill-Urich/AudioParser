#include "../include/setAlbums.h"
#include "../include/getMp3Files.h"
#include <iostream>
#include <vector>
#include <set>
#include <filesystem>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/mpegfile.h>

namespace fs = std::filesystem;
namespace TL = TagLib;

bool setAlbums(const std::string& dir_path) {
    std::vector<fs::path> mp3_files = getMp3Files(dir_path);

    if (mp3_files.empty()) {
        std::cout << "No MP3 files found in directory: " << dir_path << std::endl;
        return false;
    }

    bool all_operations_successful = true;

    // Сначала создаем все необходимые директории
    std::set<std::string> artists;
    for (const auto& file_path : mp3_files) {
        TL::MPEG::File file(file_path.c_str());
        if (!file.isValid()) continue;

        TL::Tag* tag = file.tag();
        if (!tag || tag->artist().isEmpty()) continue;

        std::string artist = tag->artist().to8Bit(true);
        artists.insert(artist);
    }

    for (const auto& artist : artists) {
        fs::path artist_dir = fs::path(dir_path) / artist;
        if (!fs::exists(artist_dir)) {
            try {
                fs::create_directory(artist_dir);
                std::cout << "Created directory: " << artist_dir << std::endl;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to create directory " << artist_dir << ": "
                        << e.what() << std::endl;
                all_operations_successful = false;
            }
        }
    }

    // Затем перемещаем файлы
    for (const auto& file_path : mp3_files) {
        TL::MPEG::File file(file_path.c_str());
        if (!file.isValid()) {
            std::cerr << "Error opening file: " << file_path << std::endl;
            all_operations_successful = false;
            continue;
        }

        TL::Tag* tag = file.tag();
        if (!tag || tag->artist().isEmpty()) {
            std::cerr << "No artist tag in file: " << file_path << std::endl;
            all_operations_successful = false;
            continue;
        }

        try {
            std::string artist = tag->artist().to8Bit(true);
            fs::path destination_dir = fs::path(dir_path) / artist;
            fs::path destination_file = destination_dir / file_path.filename();

            if (fs::equivalent(file_path, destination_file)) {
                continue;  // Файл уже на месте
            }

            if (fs::exists(destination_file)) {
                std::cerr << "File already exists: " << destination_file << std::endl;
                all_operations_successful = false;
                continue;
            }

            fs::rename(file_path, destination_file);
            std::cout << "Moved: " << file_path.filename() << " -> "
                    << destination_file << std::endl;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Failed to move file " << file_path << ": "
                    << e.what() << std::endl;
            all_operations_successful = false;
        }
    }

    return all_operations_successful;
}