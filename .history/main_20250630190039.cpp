#include <iostream>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <dirent.h>
#include <sys/stat.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/mpegfile.h>

namespace fs = std::filesystem;
using namespace std;

const int MIN_TRACKS_COUNT = 6;

bool isMp3File(const string& filename) {
    size_t dotPos = filename.find_last_of(".");
    if (dotPos != string::npos) {
        string ext = filename.substr(dotPos + 1);
        transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext == "mp3" || ext == "MP3";
    }

    return false;
}

vector<fs::path> getMp3Files(const string& directory) {
    vector<fs::path> mp3Files;
    
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        cerr << "Error: Directory doesn't exist or is not accessible: " << directory << endl;
        return mp3Files;
    }

    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file() && isMp3File(entry.path())) {
            mp3Files.push_back(entry.path());
        }
    }
    
    return mp3Files;
}

string extractMainArtist(const string& artist) {
    if (artist.empty()) return "";

    vector<string> separators = { " feat ", " feat. ", " ft ",
        " ft. ", " vs ", " vs. ", " with ", ", ", " + ", " & "};
    size_t min_pos = string::npos;

    for (const auto& sep : separators) {
        size_t pos = artist.find(sep);
        if (pos != string::npos && (min_pos == string::npos || pos < min_pos)) {
            min_pos = pos;
        }
    }

    if (min_pos != string::npos) {
        string main = artist.substr(0, min_pos);

        size_t start = main.find_first_not_of(" \t");
        if (start == string::npos) return "";

        size_t end = main.find_last_not_of(" \t");
        return main.substr(start, end - start + 1);
    }

    return artist;
}

bool processFile(const string& filepath) {
    TagLib::MPEG::File file(filepath.c_str());
    if (!file.isValid()) {
        cerr << "Error opening file: " << filepath << endl;
        return false;
    }

    TagLib::Tag *tag = file.tag();
    if (!tag || tag->artist().isEmpty()) {
        cerr << "No tags found in file: " << filepath << endl;
        return false;
    }

    string originalArtist = tag->artist().to8Bit(true);
    string newArtist = extractMainArtist(originalArtist);

    if (newArtist == originalArtist) {
        cout << "No change needed: " << filepath << endl;
        return true;
    }

    tag->setArtist(TagLib::String(newArtist, TagLib::String::UTF8));
    if (!file.save()) {
        cerr << "Save failed for: " << filepath << endl;
        return false;
    }

    cout << "Updated: " << filepath << endl;
    cout << " Old artist: " << originalArtist << endl;
    cout << " New artist: " << newArtist << endl;
    return true;
}

bool processDirectory(const string& dir_path) {
    vector<fs::path> mp3_files = getMp3Files(dir_path);

    if (mp3_files.empty()) {
        cout << "No MP3 files found in directory: " << dir_path << endl;
        return false;
    }

    sort(mp3_files.begin(), mp3_files.end(), [](const string& a, const string& b) {
        TagLib::MPEG::File fileA(a.c_str());
        TagLib::MPEG::File fileB(b.c_str());
        return fileA.tag()->artist().to8Bit(true) < fileB.tag()->artist().to8Bit(true);
    });

    vector<string> artists_to_delete;
    size_t ArtistAudioCount = 1;

    TagLib::MPEG::File previous_file(mp3_files[0].c_str());
    string previous_artist = previous_file.tag()->artist().to8Bit(true);

    for (auto it = mp3_files.begin() + 1; it != mp3_files.end(); ++it) {
        TagLib::MPEG::File current_file(it->c_str());
        if (!current_file.isValid()) {
            cerr << "Error opening file: " << *it << endl;
            continue;
        }

        string current_artist = current_file.tag()->artist().to8Bit(true);

        if (current_artist == previous_artist) {
            ArtistAudioCount++;
        } else {
            if (ArtistAudioCount <= MIN_TRACKS_COUNT) {
                artists_to_delete.push_back(previous_artist);
            }
            previous_artist = current_artist;
            ArtistAudioCount = 1;
        }
    }

    if (ArtistAudioCount <= MIN_TRACKS_COUNT) {
        artists_to_delete.push_back(previous_artist);
    }

    if (!artists_to_delete.empty()) {
        size_t deleted_count = 0;

        sort(artists_to_delete.begin(), artists_to_delete.end());
        artists_to_delete.erase(unique(artists_to_delete.begin(), artists_to_delete.end()), 
                               artists_to_delete.end());

        cout << "Artists with less than " << MIN_TRACKS_COUNT << " tracks will be deleted:" << endl;
        for (const auto& artist : artists_to_delete) {
            cout << " - " << artist << endl;
        }

        for (const auto & file_path : mp3_files) {
            TagLib::MPEG::File file(file_path.c_str());
            if (!file.isValid()) continue;

            string current_artist = file.tag()->artist().to8Bit(true);

            if (find(artists_to_delete.begin(), artists_to_delete.end(), current_artist) != artists_to_delete.end()) {
                try {
                    if (fs::remove(file_path)) {
                        cout << "Deleted: " << file_path.filename() << endl;
                        deleted_count++;
                    }
                } catch (const fs::filesystem_error& e) {
                    cerr << "Error deleting " << file_path << ": " << e.what() << endl;
                }
            }
        }
    }
    cout << "No artists with less than " << MIN_TRACKS_COUNT << " tracks found" << endl;
    return false;
}