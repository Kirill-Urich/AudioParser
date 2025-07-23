#pragma once

#include <iostream>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

std::vector<fs::path> getMp3Files(const std::string& directory);