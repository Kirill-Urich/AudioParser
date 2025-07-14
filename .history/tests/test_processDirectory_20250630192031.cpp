#include "../include/processDirectory.h"
#include "../include/constants.h"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/mpegfile.h>

namespace fs = std::filesystem;
namespace TL = TagLib;

// Вспомогательная функция для создания тестовых MP3 файлов с тегами
void createTestMp3(const fs::path& path, const std::string& artist) {
    std::ofstream(path).put(' '); // Создаем пустой файл
    
    TL::MPEG::File file(path.c_str());
    file.tag()->setArtist(artist);
    file.save();
}

TEST_CASE("processDirectory - обработка директории с MP3 файлами") {
    // Подготовка тестового окружения
    const fs::path test_dir = "test_music_dir";
    fs::create_directory(test_dir);
    
    // Создаем тестовые файлы с разными артистами
    createTestMp3(test_dir / "artist1_1.mp3", "Artist One");
    createTestMp3(test_dir / "artist1_2.mp3", "Artist One");
    createTestMp3(test_dir / "artist2_1.mp3", "Artist Two");
    createTestMp3(test_dir / "artist3_1.mp3", "Artist Three"); // Будет удален (1 трек)
    createTestMp3(test_dir / "artist4_1.mp3", "Artist Four");  // Будет удален (1 трек)
    createTestMp3(test_dir / "artist1_3.mp3", "Artist One");

    // Меняем константу для тестирования (вместо 5 треков)
    const size_t original_min_tracks = Constants::MIN_TRACKS_COUNT;
    Constants::MIN_TRACKS_COUNT = 2;

    SECTION("Удаление артистов с малым количеством треков") {
        REQUIRE(processDirectory(test_dir.string()) == true);
        
        // Проверяем, что файлы удалились корректно
        REQUIRE(fs::exists(test_dir / "artist1_1.mp3"));
        REQUIRE(fs::exists(test_dir / "artist1_2.mp3"));
        REQUIRE(fs::exists(test_dir / "artist2_1.mp3"));
        REQUIRE(fs::exists(test_dir / "artist1_3.mp3"));
        REQUIRE_FALSE(fs::exists(test_dir / "artist3_1.mp3"));
        REQUIRE_FALSE(fs::exists(test_dir / "artist4_1.mp3"));
    }

    SECTION("Обработка пустой директории") {
        const fs::path empty_dir = test_dir / "empty_subdir";
        fs::create_directory(empty_dir);
        
        REQUIRE(processDirectory(empty_dir.string()) == false);
        fs::remove_all(empty_dir);
    }

    SECTION("Обработка несуществующей директории") {
        REQUIRE(processDirectory("non_existent_directory") == false);
    }

    // Восстанавливаем оригинальное значение константы
    Constants::MIN_TRACKS_COUNT = original_min_tracks;
    
    // Очистка тестового окружения
    fs::remove_all(test_dir);
}