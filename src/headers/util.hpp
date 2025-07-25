#pragma once

#include <filesystem>

#include "sha1.hpp"

enum class FileType{BLOB, TREE};

std::string hashObject(const std::string& fileData);
std::filesystem::path findRepositoryRoot(std::filesystem::path start);
const std::string serializeFile(const std::filesystem::path& filePath, const FileType& fileType);
