#pragma once

#include <filesystem>

#include "sha1.hpp"

enum class FileType{BLOB, TREE, EMPTY};

struct blob{
    std::string hash;
    std::string fileName;
    std::string fileMode;
    FileType fileType;

    blob(std::string hash, std::string fileName, FileType fileType) 
        :  hash(hash), fileName(fileName), fileType(fileType)
    {
        switch (fileType){

            case FileType::BLOB :
                fileMode = "100644";
            break;
            case FileType::TREE :
                fileMode = "040000";
            break;
            default :
                fileMode = "NULL";
            break;
        }

    }

    blob (FileType fileType)
        : fileType(fileType){}
};

std::string hashObject(const std::string& fileData);
std::filesystem::path findRepositoryRoot(std::filesystem::path start);
void writeToObjectsFolder(const std::string& contents, const std::string& hash, const std::filesystem::path& repositoryRoot);
