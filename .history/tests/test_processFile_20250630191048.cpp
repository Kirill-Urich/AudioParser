#include "../include/processFile.h"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <taglib/fileref.h>
#include <taglib/tag.h>

namespace fs = std::filesystem;
namespace TL = TagLib;

// Вспомогательная функция для создания тестовых MP3 файлов
void createTestFile(const fs::path& path, const std::string& artist) {
    std::ofstream(path).put(' '); // Создаем пустой файл
    TL::MPEG::File file(path.c_str());
    file.tag()->setArtist(artist);
    file.save();
}

TEST_CASE("processFile - обработка MP3 файлов") {
    const fs::path test_dir = "test_process_dir";
    fs::create_directory(test_dir);

    SECTION("Корректное извлечение основного артиста") {
        const fs::path test_file = test_dir / "test.mp3";
        createTestFile(test_file, "Main Artist feat. Other");
        
        REQUIRE(processFile(test_file.string()) == true);
        
        // Проверяем что теги изменились
        TL::MPEG::File file(test_file.c_str());
        REQUIRE(file.tag()->artist().to8Bit(true) == "Main Artist");
    }

    SECTION("Файл без изменений") {
        const fs::path test_file = test_dir / "no_change.mp3";
        createTestFile(test_file, "Solo Artist");
        
        REQUIRE(processFile(test_file.string()) == true);
        
        // Проверяем что теги не изменились
        TL::MPEG::File file(test_file.c_str());
        REQUIRE(file.tag()->artist().to8Bit(true) == "Solo Artist");
    }

    SECTION("Обработка разных форматов артистов") {
        struct TestCase {
            std::string input;
            std::string expected;
        };

        const std::vector<TestCase> cases = {
            {"Artist ft. Other", "Artist"},
            {"Band & Friends", "Band"},
            {"Main VS Secondary", "Main"},
            {"A feat. B, C + D", "A"},
            {"  Trimmed  feat.  Other  ", "Trimmed"}
        };

        for (const auto& [input, expected] : cases) {
            const fs::path file = test_dir / ("test_" + input + ".mp3");
            createTestFile(file, input);
            
            REQUIRE(processFile(file.string()) == true);
            
            TL::MPEG::File f(file.c_str());
            REQUIRE(f.tag()->artist().to8Bit(true) == expected);
        }
    }

    SECTION("Ошибки обработки") {
        SECTION("Несуществующий файл") {
            REQUIRE(processFile("nonexistent.mp3") == false);
        }

        SECTION("Файл без тегов") {
            const fs::path no_tags = test_dir / "no_tags.mp3";
            std::ofstream(no_tags).put(' '); // Создаем без тегов
            
            REQUIRE(processFile(no_tags.string()) == false);
        }

        SECTION("Файл с пустым артистом") {
            const fs::path empty_artist = test_dir / "empty_artist.mp3";
            createTestFile(empty_artist, "");
            
            REQUIRE(processFile(empty_artist.string()) == false);
        }

        SECTION("Ошибка сохранения (read-only файл)") {
            const fs::path read_only = test_dir / "read_only.mp3";
            createTestFile(read_only, "Artist feat. Other");
            fs::permissions(read_only, fs::perms::owner_read);
            
            REQUIRE(processFile(read_only.string()) == false);
            fs::permissions(read_only, fs::perms::owner_write); // Восстанавливаем права
        }
    }

    // Очистка
    fs::remove_all(test_dir);
}