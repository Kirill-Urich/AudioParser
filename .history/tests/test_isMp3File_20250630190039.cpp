#include "../include/isMp3File.h"
#include <vector>
#include <catch2/catch_test_macros.hpp>
 
TEST_CASE("Проверка isMp3File") {
    std::vector<std::string> correct_mp3_files {
        "audiofile.mp3",        // базовый случай
        "AUDIOFILE.MP3",        // верхний регистр
        "AudioFile.Mp3",        // смешанный регистр
        "  audio.mp3  ",        // пробелы вокруг
        "audio .mp3",           // пробел перед точкой
        "archive.tar.mp3",      // несколько точек
        "C:/music/track.mp3",   // путь в Windows
        "/var/lib/song.mp3",    // путь в Linux
        "#хештег$.mp3"          // спецсимволы
    };

    std::vector<std::string> incorrect_mp3_files {
        "audiofile.wav",        // другое расширение
        "audiofilemp3",         // нет точки
        "audiofile.",           // точка без расширения
        ".mp3",                 // только расширение
        "mp3",                  // просто текст
        "mp3.txt",              // подстрока
        "",                     // пустая строка
        "audiofile.mp3 ",       // пробел после
        "audiofile.mp3\0"       // нулевой символ
    };
    SECTION("Корректные MP3-файлы") {
        for (const auto& it : correct_mp3_files)
            REQUIRE(isMp3File(it) == true);
    }
    
    SECTION("Некорректные файлы") {
        for (const auto& it : incorrect_mp3_files)
            REQUIRE(isMp3File(it) == false);
    }
}

а теперь?