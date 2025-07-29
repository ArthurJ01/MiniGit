#pragma once

#include <filesystem>

#include "sha1.hpp"

enum class FileType{BLOB, TREE};

struct blob{
    std::string hash;
    std::string fileName;
    std::string fileMode;

    blob(std::string hash, std::string fileName, FileType fileType) 
        :  hash(hash), fileName(fileName)
    {
        switch (fileType){

            case FileType::BLOB :
                fileMode = "100644";
            break;
            case FileType::TREE :
                fileMode = "040000";
            break;
        }

    }
};

std::string hashObject(const std::string& fileData);
std::filesystem::path findRepositoryRoot(std::filesystem::path start);
