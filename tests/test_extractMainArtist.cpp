#include "../include/extractMainArtist.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("extractMainArtist - извлечение основного исполнителя") {
    SECTION("Базовые случаи с разными разделителями") {
        REQUIRE(extractMainArtist("Artist feat. Other") == "Artist");
        REQUIRE(extractMainArtist("Artist ft. Other") == "Artist");
        REQUIRE(extractMainArtist("Artist vs Other") == "Artist");
        REQUIRE(extractMainArtist("Artist with Other") == "Artist");
        REQUIRE(extractMainArtist("Artist, Other") == "Artist");
        REQUIRE(extractMainArtist("Artist + Other") == "Artist");
        REQUIRE(extractMainArtist("Artist & Other") == "Artist");
    }

    SECTION("Разные регистры разделителей") {
        REQUIRE(extractMainArtist("Artist Feat. Other") == "Artist");
        REQUIRE(extractMainArtist("Artist FT Other") == "Artist");
        REQUIRE(extractMainArtist("Artist VS. Other") == "Artist");
        REQUIRE(extractMainArtist("Artist With Other") == "Artist");
    }

    SECTION("Множественные разделители") {
        REQUIRE(extractMainArtist("Artist feat. Other vs Another") == "Artist");
        REQUIRE(extractMainArtist("Artist & Other + Third") == "Artist");
    }

    SECTION("Пробелы вокруг имени") {
        REQUIRE(extractMainArtist("  Artist  feat. Other") == "Artist");
        REQUIRE(extractMainArtist("Artist   ft.  Other") == "Artist");
        REQUIRE(extractMainArtist("Artist\tvs\tOther") == "Artist");
    }

    SECTION("Строки без разделителей") {
        REQUIRE(extractMainArtist("Solo Artist") == "Solo Artist");
        REQUIRE(extractMainArtist("Artist") == "Artist");
        REQUIRE(extractMainArtist("  Trimmed Artist  ") == "Trimmed Artist");
    }

    SECTION("Граничные случаи") {
        REQUIRE(extractMainArtist("") == "");
        REQUIRE(extractMainArtist("feat. Only Other") == "");
        REQUIRE(extractMainArtist("   ") == "");
        REQUIRE(extractMainArtist("Artist &") == "Artist");
        REQUIRE(extractMainArtist("Artist,") == "Artist");
    }

    SECTION("Специальные символы в имени") {
        REQUIRE(extractMainArtist("A$AP Rocky feat. Other") == "A$AP Rocky");
        REQUIRE(extractMainArtist("Kendrick Lamar (K.Dot) vs Other") == "Kendrick Lamar (K.Dot)");
        REQUIRE(extractMainArtist("Dr. Dre ft. Snoop") == "Dr. Dre");
    }

    SECTION("Unicode символы") {
        REQUIRE(extractMainArtist("Ёлка feat. Грибы") == "Ёлка");
        REQUIRE(extractMainArtist("周杰伦 & 蔡依林") == "周杰伦");
        REQUIRE(extractMainArtist("Rammstein vs. Müller") == "Rammstein");
    }
};