#include "../include/setArtistAlbums.h"
#include "../include/getMp3Files.h"
#include <iostream>
#include <filesystem>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/mpegfile.h>

namespace fs = std::filesystem;
namespace TL = TagLib;

bool setArtistAlbums(const std::string& dir_path) {
    try {
        std::cout << "\n=== Starting organization in: " << fs::absolute(dir_path) << " ===\n";

        // Проверка директории
        if (!fs::exists(dir_path)) {
            std::cerr << "ERROR: Directory doesn't exist: " << dir_path << std::endl;
            return false;
        }
        if (!fs::is_directory(dir_path)) {
            std::cerr << "ERROR: Path is not a directory: " << dir_path << std::endl;
            return false;
        }

        // Получаем файлы
        auto mp3_files = getMp3Files(dir_path);
        std::cout << "Found " << mp3_files.size() << " MP3 files\n";

        if (mp3_files.empty()) {
            std::cout << "No MP3 files to process\n";
            return true;
        }

        bool all_success = true;

        for (const auto& file_path : mp3_files) {
            std::cout << "\nProcessing: " << file_path.filename() << std::endl;

            try {
                // Чтение тегов
                TL::MPEG::File file(file_path.c_str());
                if (!file.isValid()) {
                    std::cerr << "ERROR: Invalid MP3 file\n";
                    all_success = false;
                    continue;
                }

                auto tag = file.tag();
                if (!tag) {
                    std::cerr << "ERROR: No tags found\n";
                    all_success = false;
                    continue;
                }

                // Получаем артиста и альбом
                std::string artist = tag->artist().isEmpty() ? 
                    "Unknown Artist" : tag->artist().to8Bit(true);
                std::string album = tag->album().isEmpty() ? 
                    "Unknown Album" : tag->album().to8Bit(true);

                std::cout << "Artist: " << artist << " | Album: " << album << std::endl;

                // Формируем новые пути
                fs::path new_dir = fs::absolute(dir_path) / artist / album;
                fs::path new_path = new_dir / file_path.filename();

                std::cout << "Target path: " << new_path << std::endl;

                // Пропускаем если уже на месте
                if (fs::equivalent(file_path, new_path)) {
                    std::cout << "File already in correct location\n";
                    continue;
                }

                // Создаём директории (рекурсивно)
                if (!fs::exists(new_dir)) {
                    std::cout << "Creating directory: " << new_dir << std::endl;
                    if (!fs::create_directories(new_dir)) {
                        std::cerr << "ERROR: Failed to create directory\n";
                        all_success = false;
                        continue;
                    }
                }

                // Перемещаем файл
                std::cout << "Moving file...\n";
                fs::rename(file_path, new_path);
                std::cout << "Successfully moved\n";

            } catch (const fs::filesystem_error& e) {
                std::cerr << "Filesystem ERROR: " << e.what() << std::endl;
                all_success = false;
            } catch (const std::exception& e) {
                std::cerr << "ERROR: " << e.what() << std::endl;
                all_success = false;
            }
        }

        std::cout << "\n=== Operation completed: " << (all_success ? "SUCCESS" : "HAD ERRORS") << " ===\n";
        return all_success;

    } catch (const std::exception& e) {
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        return false;
    }
}