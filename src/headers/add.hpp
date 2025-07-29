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
void writeBlob(const std::string& contents, const std::string& hash, const std::filesystem::path& repositoryRoot);