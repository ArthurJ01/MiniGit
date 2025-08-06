#pragma once

#include <filesystem>
#include <sstream>
#include <fstream>
#include <stack>
#include <iostream>

#include "util.hpp"

enum class CheckoutTargetType {
    BRANCH,
    COMMIT
};

struct treeBlob {
    std::string hash;
    std::string fileName;
    std::string fileMode;
    std::filesystem::path fullPath;

    treeBlob(std::string hash, std::string fileName, std::string fileMode, std::filesystem::path fullPath)
        : hash(hash), fileName(fileName), fileMode(fileMode), fullPath(fullPath) {}
};

void checkout(char* argv[]);

bool areThereUncommitedChanges(const std::filesystem::path& repositoryRoot);
std::stack<treeBlob>& findAllFiles(std::stack<treeBlob>& listOfBlobs, const std::filesystem::path& treeObjectPath, const std::filesystem::path& realFilePath);
std::string getCommitHash(const std::filesystem::path& repositoryRoot, const std::string& checkoutTarget);
std::string getTreeObjectHash(const std::filesystem::path& repositoryRoot, const std::string& commitHash);
void deleteAllContentInRepository(const std::filesystem::path& repositoryRoot);
