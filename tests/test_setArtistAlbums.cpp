#include "../include/setArtistAlbums.h"
#include "../include/getMp3Files.h"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/mpegfile.h>


namespace fs = std::filesystem;
namespace TL = TagLib;

// Вспомогательная функция для создания тестового MP3 файла с тегами
void createTestMp3(const fs::path& path, 
                  const std::string& artist, 
                  const std::string& album,
                  const std::string& title) {
    std::ofstream file(path, std::ios::binary);
    file << "Dummy MP3 content"; // Минимальное содержимое файла
    
    TL::MPEG::File f(path.c_str());
    auto tag = f.tag();
    tag->setArtist(artist);
    tag->setAlbum(album);
    tag->setTitle(title);
    f.save();
}

TEST_CASE("setArtistAlbums organizes files correctly", "[setArtistAlbums]") {
    // Создаем временную директорию для тестов
    fs::path temp_dir = fs::temp_directory_path() / "mp3_organizer_test";
    fs::create_directory(temp_dir);
    
    SECTION("Organizes files by artist and album") {
        // Создаем тестовые файлы
        createTestMp3(temp_dir / "song1.mp3", "Artist1", "Album1", "Song1");
        createTestMp3(temp_dir / "song2.mp3", "Artist1", "Album1", "Song2");
        createTestMp3(temp_dir / "song3.mp3", "Artist1", "Album2", "Song3");
        createTestMp3(temp_dir / "song4.mp3", "Artist2", "Album1", "Song4");
        
        REQUIRE(setArtistAlbums(temp_dir.string()));
        
        // Проверяем структуру директорий
        CHECK(fs::exists(temp_dir / "Artist1" / "Album1" / "song1.mp3"));
        CHECK(fs::exists(temp_dir / "Artist1" / "Album1" / "song2.mp3"));
        CHECK(fs::exists(temp_dir / "Artist1" / "Album2" / "song3.mp3"));
        CHECK(fs::exists(temp_dir / "Artist2" / "Album1" / "song4.mp3"));
    }
    
    SECTION("Handles files without tags") {
        // Создаем файл без тегов
        std::ofstream(temp_dir / "untagged.mp3") << "Dummy content";
        
        REQUIRE(setArtistAlbums(temp_dir.string()));
        
        // Проверяем, что файл перемещен в Unknown directories
        CHECK(fs::exists(temp_dir / "Unknown Artist" / "Unknown Album" / "untagged.mp3"));
    }
    
    SECTION("Skips already organized files") {
        // Создаем уже организованную структуру
        fs::create_directory(temp_dir / "Artist1");
        fs::create_directory(temp_dir / "Artist1" / "Album1");
        createTestMp3(temp_dir / "Artist1" / "Album1" / "song1.mp3", "Artist1", "Album1", "Song1");
        
        auto original_path = temp_dir / "Artist1" / "Album1" / "song1.mp3";
        auto original_size = fs::file_size(original_path);
        
        REQUIRE(setArtistAlbums(temp_dir.string()));
        
        // Проверяем, что файл не был перемещен повторно
        CHECK(fs::exists(original_path));
        CHECK(fs::file_size(original_path) == original_size);
    }
    
    SECTION("Returns false for non-existent directory") {
        REQUIRE_FALSE(setArtistAlbums("/nonexistent/path"));
    }
    
    SECTION("Returns true when no mp3 files found") {
        REQUIRE(setArtistAlbums(temp_dir.string()));
    }
    
    // Очистка после тестов
    fs::remove_all(temp_dir);
}

TEST_CASE("setArtistAlbums handles errors gracefully", "[setArtistAlbums]") {
    SECTION("Returns false when directory is not accessible") {
        fs::path restricted_dir = "/root/mp3_test";
        REQUIRE_FALSE(setArtistAlbums(restricted_dir.string()));
    }
    
    // Можно добавить больше тестов для обработки ошибок
    // например, когда файл занят другим процессом
}