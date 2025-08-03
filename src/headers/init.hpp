#pragma once

#include <filesystem>

bool createOrOverwriteFile(const std::filesystem::path& filePath);
void init(char* argv[]);