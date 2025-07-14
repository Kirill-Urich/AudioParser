

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <directory>" << endl;
        return 1;
    }

    fs::path directory(argv[1]);
    vector<fs::path> mp3Files = getMp3Files(directory);

    if (mp3Files.empty()) {
        cout << "No MP3 files found in directory: " << directory << endl;
        return 0;
    }

    cout << "Found " << mp3Files.size() << " MP3 files in " << directory << endl;
    cout << "Processing artist tags..." << endl;

    int changed = 0;

    for (const auto& filepath : mp3Files) {
        if (processFile(filepath)) {
            changed++;
        }
    }

    cout << "\nProcessing complete!" << endl;
    cout << "Total processed: " << mp3Files.size() << endl;
    cout << "Files changed: " << changed << endl;
    cout << "Files unchanged: " << mp3Files.size() - changed << endl;


    cout << "Scanning for artists with less than " << MIN_TRACKS_COUNT << " tracks..." << endl;

    if (processDirectory(directory)) {
        cout << "Cleanup completed." << endl;
    } else {
        cout << "No files were deleted." << endl;
    }

    return 0;
}