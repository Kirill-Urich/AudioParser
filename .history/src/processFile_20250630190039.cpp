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