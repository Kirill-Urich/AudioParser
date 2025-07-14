#include "../include/processFile.h"
#include "../include/extractMainArtist.h"
#include <iostream>
#include <algorithm>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/mpegfile.h>

bool processFile(const std::string& filepath) {
    TagLib::MPEG::File file(filepath.c_str());
    if (!file.isValid()) {
        std::cerr << "Error opening file: " << filepath << std::endl;
        return false;
    }

    TagLib::Tag *tag = file.tag();
    if (!tag || tag->artist().isEmpty()) {
        std::cerr << "No tags found in file: " << filepath << std::endl;
        return false;
    }

    std::string originalArtist = tag->artist().to8Bit(true);
    std::string newArtist = extractMainArtist(originalArtist);

    if (newArtist == originalArtist) {
        std::cout << "No change needed: " << filepath << std::endl;
        return true;
    }

    tag->setArtist(TagLib::String(newArtist, TagLib::String::UTF8));
    if (!file.save()) {
        std::cerr << "Save failed for: " << filepath << std::endl;
        return false;
    }

    std::cout << "Updated: " << filepath << std::endl;
    std::cout << " Old artist: " << originalArtist << std::endl;
    std::cout << " New artist: " << newArtist << std::endl;
    return true;
}


напиши тест для этой функции