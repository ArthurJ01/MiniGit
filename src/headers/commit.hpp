#pragma once

#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <vector>

#include "util.hpp"

void commit (char* argv[]);
std::unordered_map<std::filesystem::path, std::string> loadIndex(const std::filesystem::path& indexPath);
blob createCommitTree(std::filesystem::path filePath, std::filesystem::path repositoryRoot, const std::unordered_map<std::filesystem::path, std::string>& indexMap, const std::unordered_map<std::filesystem::path, std::string>& lastCommitMap);
void updateHeadFile(const std::filesystem::path& repositoryRoot, const std::string& hash);