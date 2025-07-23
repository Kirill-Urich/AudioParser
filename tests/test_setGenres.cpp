#include "../include/setGenres.h"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/mpegfile.h>

namespace fs = std::filesystem;
namespace TL = TagLib;

// Вспомогательная функция для создания тестовых MP3 файлов
void createTestMp3(const fs::path& path, const std::string& artist, const std::string& genre = "") {
    std::ofstream(path).put(' '); // Создаем пустой файл
    TL::MPEG::File file(path.c_str());
    file.tag()->setArtist(artist);
    if (!genre.empty()) {
        file.tag()->setGenre(genre);
    }
    file.save();
}

// Вспомогательная функция для создания ArtistGenres.txt
void createArtistGenres(const fs::path& path, const std::string& content) {
    std::ofstream file(path);
    file << content;
    file.close();
}

TEST_CASE("setGenres - установка жанров для MP3 файлов") {
    // Подготовка тестового окружения
    const fs::path test_dir = "test_music_dir";
    const fs::path genres_file = "ArtistGenres.txt";
    fs::create_directory(test_dir);

    // Содержимое тестового файла жанров
    const std::string genres_content = 
        "Artist1 - Rock\n"
        "Artist2 - Pop\n"
        "  Artist3   -   Hip Hop  \n"
        "Duplicate - Jazz\n"
        "Duplicate - Classical\n";

    createArtistGenres(genres_file, genres_content);

    // Создаем тестовые MP3 файлы
    createTestMp3(test_dir / "song1.mp3", "Artist1", "Old Genre");
    createTestMp3(test_dir / "song2.mp3", "Artist2");
    createTestMp3(test_dir / "song3.mp3", "Artist3");
    createTestMp3(test_dir / "song4.mp3", "UnknownArtist");
    createTestMp3(test_dir / "song5.mp3", "Duplicate");

    SECTION("Корректная установка жанров") {
        REQUIRE(setGenres(test_dir.string()) == true);

        // Проверяем установленные жанры
        TL::MPEG::File file1((test_dir / "song1.mp3").c_str());
        REQUIRE(file1.tag()->genre().to8Bit(true) == "Rock");

        TL::MPEG::File file2((test_dir / "song2.mp3").c_str());
        REQUIRE(file2.tag()->genre().to8Bit(true) == "Pop");

        TL::MPEG::File file3((test_dir / "song3.mp3").c_str());
        REQUIRE(file3.tag()->genre().to8Bit(true) == "Hip Hop");
    }

    SECTION("Обработка ошибок и предупреждений") {
        REQUIRE(setGenres(test_dir.string()) == true);

        // Проверяем обработку дубликатов
        TL::MPEG::File file5((test_dir / "song5.mp3").c_str());
        REQUIRE((file5.tag()->genre().to8Bit(true) == "Jazz" || 
                file5.tag()->genre().to8Bit(true) == "Classical"));

        // Файл с неизвестным артистом не должен измениться
        TL::MPEG::File file4((test_dir / "song4.mp3").c_str());
        REQUIRE(file4.tag()->genre().isEmpty());
    }

    SECTION("Обработка несуществующей директории") {
        REQUIRE(setGenres("non_existent_directory") == false);
    }

    SECTION("Обработка отсутствующего файла жанров") {
        fs::remove(genres_file);
        REQUIRE(setGenres(test_dir.string()) == false);
    }

    SECTION("Обработка пустой директории") {
        const fs::path empty_dir = test_dir / "empty_subdir";
        fs::create_directory(empty_dir);
        
        REQUIRE(setGenres(empty_dir.string()) == false);
        fs::remove_all(empty_dir);
    }

    // Очистка
    fs::remove(genres_file);
    fs::remove_all(test_dir);
}