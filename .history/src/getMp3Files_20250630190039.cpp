std::vector<fs::path> getMp3Files(const std::string& directory) {
    std::vector<fs::path> mp3Files;

    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Error: Directory doesn't exist or is not accessible: " << directory << std::endl;
        return mp3Files;
    }

    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file() && isMp3File(entry.path())) {
            mp3Files.push_back(entry.path());
        }
    }

    return mp3Files;
}