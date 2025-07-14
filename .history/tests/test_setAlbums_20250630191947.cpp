#include "../include/setAlbums.h"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/mpegfile.h>

namespace fs = std::filesystem;
namespace TL = TagLib;

// Вспомогательная функция для создания тестового MP3 файла
void createTestMp3(const fs::path& path, const std::string& artist = "") {
    std::ofstream(path).put(' ');   // Создаем пустой файл
    TL::MPEG::File file(path.c_str());
    file.tag()->setArtist(artist);
    file.save();
}

TEST_CASE("setAlbums - организация MP3 файлов по артистам в текущей директории") {
    // Подготовка тестового окружения
    const fs::path test_dir = "test_dir";
    fs::create_directory(test_dir);

    // Создаем тестовые MP3 файлы
    createTestMp3(test_dir / "song1.mp3", "Artist A");
    createTestMp3(test_dir / "song2.mp3", "Artist B");
    createTestMp3(test_dir / "song3.mp3", "Artist A");
    createTestMp3(test_dir / "no_artist.mp3", "");
    createTestMp3(test_dir / "song4.mp3", "Artist C");

    SECTION("Успешная организация файлов") {
        REQUIRE(setAlbums(test_dir.string()) == true);

        // Проверяем созданные директории
        REQUIRE(fs::exists(test_dir / "Artist A"));
        REQUIRE(fs::exists(test_dir / "Artist B"));
        REQUIRE(fs::exists(test_dir / "Artist C"));

        // Проверяем перемещенные файлы
        REQUIRE(fs::exists(test_dir / "Artist A" / "song1.mp3"));
        REQUIRE(fs::exists(test_dir / "Artist B" / "song2.mp3"));
        REQUIRE(fs::exists(test_dir / "Artist A" / "song3.mp3"));
        REQUIRE(fs::exists(test_dir / "Artist C" / "song4.mp3"));

        // Файл без артиста должен остаться в исходной директории
        REQUIRE(fs::exists(test_dir / "no_artist.mp3"));
    }

    SECTION("Обработка ошибок при дублировании файлов") {
        // Создаем папку артиста заранее
        fs::create_directory(test_dir / "Artist A");
        // Создаем файл с таким же именем
        createTestMp3(test_dir / "Artist A" / "song1.mp3", "Artist A");

        REQUIRE(setAlbums(test_dir.string()) == false);

        // Проверяем, что другие файлы переместились, несмотря на ошибку
        REQUIRE(fs::exists(test_dir / "Artist B" / "song2.mp3"));
    }

    SECTION("Пустая директория") {
        const fs::path empty_dir = test_dir / "empty";
        fs::create_directory(empty_dir);

        REQUIRE(setAlbums(empty_dir.string()) == false);
        fs::remove_all(empty_dir);
    }

    SECTION("Несуществующая директория") {
        REQUIRE(setAlbums("non_existent_dir") == false);
    }

    // Очистка тестового окружения
    fs::remove_all(test_dir);
}