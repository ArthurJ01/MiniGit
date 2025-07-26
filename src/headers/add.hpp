#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>

#include "util.hpp"

void add(char* argv[]);
blob addToObjectsFolder(const std::filesystem::path& filePath, const std::filesystem::path& objectFolderPath);