#pragma once

#include <filesystem>

void createOrOverwriteFile(const std::filesystem::path& filePath);
void init(char* argv[]);