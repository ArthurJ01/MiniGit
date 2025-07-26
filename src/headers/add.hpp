#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>

#include "util.hpp"

void add(char* argv[]);
void addToIndexFile(const std::filesystem::path& filePath, const std::string& hash, const std::filesystem::path& repositoryRoot);
blob addToObjectsFolder(const std::filesystem::path& filePath, const std::filesystem::path& objectFolderPath);
blob writeBlob(const std::filesystem::path& filePath, const std::string& contents, const std::string& hash, const std::filesystem::path& repositoryRoot);