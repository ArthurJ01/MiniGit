#pragma once

#include <filesystem>

#include "sha1.hpp"

enum class FileType{BLOB, TREE};

struct blob{
    std::string hash;
    std::string fileName;
    std::string fileTypeStr;

    blob(std::string hash, std::string fileName, FileType fileType) 
        :  hash(hash), fileName(fileName)
    {
        switch (fileType){

            case FileType::BLOB :
                fileTypeStr = "blob";
            break;
            case FileType::TREE :
                fileTypeStr = "tree";
            break;
        }

    }
};

std::string hashObject(const std::string& fileData);
std::filesystem::path findRepositoryRoot(std::filesystem::path start);
const std::string serializeFile(const std::filesystem::path& filePath);
