#include "../include/setArtistAlbums.h"
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

bool setArtistAlbums(const std::string& dir_path) {
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

                // Получаем теги
                auto tag = file.tag();
                if (!tag) {
                    std::cerr << "No tags in file: " << file_path << std::endl;
                    all_success = false;
                    continue;
                }

                // Получаем имя исполнителя (если нет - используем "Unknown Artist")
                std::string artist = tag->artist().isEmpty() ? 
                    "Unknown Artist" : tag->artist().to8Bit(true);

                // Получаем название альбома (если нет - используем "Unknown Album")
                std::string album = tag->album().isEmpty() ? 
                    "Unknown Album" : tag->album().to8Bit(true);

                // Создаем пути для новой структуры
                fs::path artist_dir = fs::path(dir_path) / artist;
                fs::path album_dir = artist_dir / album;
                fs::path new_path = album_dir / file_path.filename();

                // Пропускаем если файл уже в нужной директории
                if (file_path == new_path) {
                    continue;
                }

                // Создаем директорию исполнителя если нужно
                if (!fs::exists(artist_dir)) {
                    fs::create_directory(artist_dir);
                    std::cout << "Created artist directory: " << artist_dir << std::endl;
                }

                // Создаем директорию альбома если нужно
                if (!fs::exists(album_dir)) {
                    fs::create_directory(album_dir);
                    std::cout << "Created album directory: " << album_dir << std::endl;
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