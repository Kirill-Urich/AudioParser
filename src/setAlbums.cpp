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
    try {
        // Проверяем существование директории
        if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
            std::cerr << "Directory doesn't exist or is not accessible: " << dir_path << std::endl;
            return false;
        }

        // Получаем список MP3 файлов
        auto mp3_files = getMp3Files(dir_path);
        if (mp3_files.empty()) {
            std::cout << "No MP3 files found in directory: " << dir_path << std::endl;
            return false;
        }

        bool all_success = true;

        for (const auto& file_path : mp3_files) {
            try {
                // Читаем метаданные файла
                TL::MPEG::File file(file_path.c_str());
                if (!file.isValid()) {
                    std::cerr << "Error opening file: " << file_path << std::endl;
                    all_success = false;
                    continue;
                }

                // Получаем имя исполнителя
                auto tag = file.tag();
                if (!tag || tag->artist().isEmpty()) {
                    std::cerr << "No artist tag in file: " << file_path << std::endl;
                    all_success = false;
                    continue;
                }

                std::string artist = tag->artist().to8Bit(true);
                fs::path artist_dir = fs::path(dir_path) / artist;
                fs::path new_path = artist_dir / file_path.filename();

                // Пропускаем если файл уже в нужной директории
                if (file_path == new_path) {
                    continue;
                }

                // Создаем директорию если нужно
                if (!fs::exists(artist_dir)) {
                    fs::create_directory(artist_dir);
                    std::cout << "Created directory: " << artist_dir << std::endl;
                }

                // Перемещаем файл
                fs::rename(file_path, new_path);
                std::cout << "Moved: " << file_path.filename() << " -> " << new_path << std::endl;

            } catch (const std::exception& e) {
                std::cerr << "Error processing file " << file_path << ": " << e.what() << std::endl;
                all_success = false;
            }
        }

        return all_success;

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return false;
    }
}