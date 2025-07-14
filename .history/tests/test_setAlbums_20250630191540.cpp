#include "../include/setAlbums.h"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/mpegfile.h>

namespace fs = std::filesystem;
namespace TL = TagLib;

// Вспомогательная функция для создания тестовых MP3 файлов
void createTestMp3(const fs::path& path, const std::string& artist) {
    std::ofstream(path).put(' '); // Создаем пустой файл
    TL::MPEG::File file(path.c_str());
    file.tag()->setArtist(artist);
    file.save();
}

TEST_CASE("setAlbums - организация MP3 файлов по артистам") {
    // Подготовка тестового окружения
    const fs::path src_dir = "test_src_dir";
    const fs::path dest_dir = "test_dest_dir";
    fs::create_directory(src_dir);
    fs::create_directory(dest_dir);

    // Создаем тестовые MP3 файлы
    createTestMp3(src_dir / "song1.mp3", "Artist A");
    createTestMp3(src_dir / "song2.mp3", "Artist B");
    createTestMp3(src_dir / "song3.mp3", "Artist A"); // Второй трек того же артиста
    createTestMp3(src_dir / "no_artist.mp3", "");     // Файл без артиста
    createTestMp3(src_dir / "song4.mp3", "Artist C");

    SECTION("Успешная организация файлов") {
        REQUIRE(setAlbums(src_dir.string(), dest_dir.string()) == true);
        
        // Проверяем созданные директории
        REQUIRE(fs::exists(dest_dir / "Artist A"));
        REQUIRE(fs::exists(dest_dir / "Artist B"));
        REQUIRE(fs::exists(dest_dir / "Artist C"));
        
        // Проверяем перемещенные файлы
        REQUIRE(fs::exists(dest_dir / "Artist A" / "song1.mp3"));
        REQUIRE(fs::exists(dest_dir / "Artist B" / "song2.mp3"));
        REQUIRE(fs::exists(dest_dir / "Artist A" / "song3.mp3"));
        REQUIRE(fs::exists(dest_dir / "Artist C" / "song4.mp3"));
        
        // Файл без артиста должен остаться в исходной директории
        REQUIRE(fs::exists(src_dir / "no_artist.mp3"));
    }

    SECTION("Обработка ошибок") {
        // Создаем ситуацию с конфликтом имен
        fs::create_directory(dest_dir / "Artist A");
        std::ofstream(dest_dir / "Artist A" / "song1.mp3").put(' ');
        
        REQUIRE(setAlbums(src_dir.string(), dest_dir.string()) == false);
        
        // Проверяем что другие файлы переместились несмотря на ошибку
        REQUIRE(fs::exists(dest_dir / "Artist B" / "song2.mp3"));
    }

    SECTION("Пустая исходная директория") {
        const fs::path empty_dir = src_dir / "empty";
        fs::create_directory(empty_dir);
        
        REQUIRE(setAlbums(empty_dir.string(), dest_dir.string()) == false);
        fs::remove_all(empty_dir);
    }

    SECTION("Несуществующая исходная директория") {
        REQUIRE(setAlbums("non_existent_dir", dest_dir.string()) == false);
    }

    // Очистка тестового окружения
    fs::remove_all(src_dir);
    fs::remove_all(dest_dir);
}