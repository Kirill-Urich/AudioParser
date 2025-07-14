#include "../include/getMp3Files.h"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <vector>

namespace fs = std::filesystem;

TEST_CASE("getMp3Files - поиск MP3 файлов в директории") {
    // Создаем временную директорию для тестов
    const fs::path test_dir = "test_mp3_dir";
    fs::create_directory(test_dir);
    
    // Создаем тестовые файлы
    const std::vector<fs::path> test_files = {
        test_dir / "song1.mp3",
        test_dir / "SONG2.MP3",
        test_dir / "not_an_mp3.wav",
        test_dir / "fake.mp3.txt",
        test_dir / "empty.mp3"
    };

    // Создаем файлы на диске
    for (const auto& file : test_files) {
        std::ofstream(file).put(' '); // Создаем пустой файл
    }

    SECTION("Корректное нахождение MP3 файлов") {
        auto result = getMp3Files(test_dir.string());
        
        // Ожидаемые MP3 файлы (должны быть найдены)
        std::vector<fs::path> expected = {
            test_dir / "song1.mp3",
            test_dir / "SONG2.MP3"
        };
        
        REQUIRE(result.size() == expected.size());
        for (const auto& file : expected) {
            REQUIRE(std::find(result.begin(), result.end(), file) != result.end());
        }
    }

    SECTION("Пропускает не-MP3 файлы") {
        auto result = getMp3Files(test_dir.string());
        
        // Файлы, которые НЕ должны быть в результате
        std::vector<fs::path> not_expected = {
            test_dir / "not_an_mp3.wav",
            test_dir / "fake.mp3.txt"
        };
        
        for (const auto& file : not_expected) {
            REQUIRE(std::find(result.begin(), result.end(), file) == result.end());
        }
    }

    SECTION("Обработка несуществующей директории") {
        auto result = getMp3Files("non_existent_directory");
        REQUIRE(result.empty());
    }

    SECTION("Обработка пустой директории") {
        const fs::path empty_dir = test_dir / "empty_subdir";
        fs::create_directory(empty_dir);
        
        auto result = getMp3Files(empty_dir.string());
        REQUIRE(result.empty());
        
        fs::remove_all(empty_dir);
    }

    // Убираем тестовую директорию после завершения тестов
    fs::remove_all(test_dir);
};