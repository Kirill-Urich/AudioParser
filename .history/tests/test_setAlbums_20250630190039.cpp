#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include "../include/setAlbums.h"
#include <taglib/fileref.h>
#include <taglib/tag.h>

namespace fs = std::filesystem;

// Вспомогательная функция для создания тестового MP3 файла
void create_test_mp3(const fs::path& path, 
                    const std::string& artist = "", 
                    const std::string& title = "",
                    const std::string& album = "") {
    std::ofstream file(path, std::ios::binary);
    file << "Dummy MP3 content";
    file.close();

    TagLib::MPEG::File file_tag(path.c_str(), true);
    auto* tag = file_tag.tag();
    if (!artist.empty()) tag->setArtist(artist);
    if (!title.empty()) tag->setTitle(title);
    if (!album.empty()) tag->setAlbum(album);
    file_tag.save();
}

TEST_CASE("setAlbums creates artist directories", "[setAlbums]") {
    const auto test_dir = fs::temp_directory_path() / "mp3_test";
    fs::create_directories(test_dir);
    
    const auto src_dir = test_dir / "src";
    fs::create_directories(src_dir);
    create_test_mp3(src_dir / "song1.mp3", "Artist A", "Song 1");
    create_test_mp3(src_dir / "song2.mp3", "Artist B", "Song 2");

    const auto dest_dir = test_dir / "dest";
    fs::create_directories(dest_dir);

    REQUIRE(setAlbums(src_dir.string(), dest_dir.string()) == true);
    
    CHECK(fs::exists(dest_dir / "Artist A"));
    CHECK(fs::exists(dest_dir / "Artist B"));
    CHECK(fs::exists(dest_dir / "Artist A" / "song1.mp3"));
    CHECK(fs::exists(dest_dir / "Artist B" / "song2.mp3"));

    fs::remove_all(test_dir);
}

TEST_CASE("setAlbums skips files without artist tag", "[setAlbums]") {
    const auto test_dir = fs::temp_directory_path() / "mp3_test_noartist";
    fs::create_directories(test_dir);
    
    const auto src_dir = test_dir / "src";
    fs::create_directories(src_dir);
    create_test_mp3(src_dir / "good.mp3", "Good Artist", "Good Song");
    create_test_mp3(src_dir / "no_artist.mp3", "", "No Artist");

    const auto dest_dir = test_dir / "dest";
    fs::create_directories(dest_dir);

    REQUIRE(setAlbums(src_dir.string(), dest_dir.string()) == false);
    
    CHECK(fs::exists(dest_dir / "Good Artist" / "good.mp3"));
    CHECK(fs::exists(src_dir / "no_artist.mp3")); // Должен остаться в исходной директории

    fs::remove_all(test_dir);
}

TEST_CASE("setAlbums handles existing destination files", "[setAlbums]") {
    const auto test_dir = fs::temp_directory_path() / "mp3_test_conflict";
    fs::create_directories(test_dir);
    
    const auto src_dir = test_dir / "src";
    fs::create_directories(src_dir);
    create_test_mp3(src_dir / "song.mp3", "Conflict Artist", "Conflict Song");

    const auto dest_dir = test_dir / "dest";
    fs::create_directories(dest_dir);
    create_test_mp3(dest_dir / "Conflict Artist" / "song.mp3", "Conflict Artist", "Conflict Song");

    REQUIRE(setAlbums(src_dir.string(), dest_dir.string()) == false);
    
    CHECK(fs::exists(src_dir / "song.mp3")); // Файл должен остаться на месте
    CHECK(fs::exists(dest_dir / "Conflict Artist" / "song.mp3"));

    fs::remove_all(test_dir);
}

TEST_CASE("setAlbums returns false for empty directory", "[setAlbums]") {
    const auto test_dir = fs::temp_directory_path() / "mp3_test_empty";
    fs::create_directories(test_dir);
    
    const auto src_dir = test_dir / "src";
    fs::create_directories(src_dir);

    const auto dest_dir = test_dir / "dest";
    fs::create_directories(dest_dir);

    REQUIRE(setAlbums(src_dir.string(), dest_dir.string()) == false);

    fs::remove_all(test_dir);
}

TEST_CASE("setAlbums handles read-only destination", "[setAlbums]") {
    const auto test_dir = fs::temp_directory_path() / "mp3_test_readonly";
    fs::create_directories(test_dir);
    
    const auto src_dir = test_dir / "src";
    fs::create_directories(src_dir);
    create_test_mp3(src_dir / "song.mp3", "Readonly Artist", "Readonly Song");

    const auto dest_dir = test_dir / "dest";
    fs::create_directories(dest_dir);
    
    // Делаем директорию только для чтения
    fs::permissions(dest_dir, fs::perms::owner_read | fs::perms::owner_exec);

    REQUIRE(setAlbums(src_dir.string(), dest_dir.string()) == false);
    
    // Восстанавливаем права для очистки
    fs::permissions(dest_dir, fs::perms::owner_all);
    fs::remove_all(test_dir);
}