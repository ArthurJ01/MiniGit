#include <iostream>
#include <fstream>
#include <filesystem>
#include <string.h>
#include <direct.h>

#include "init.hpp"


bool is_relative_path(const char* path) {

    if(path != nullptr && strncmp(path, "./", 2) == 0){
        return true;
    }
    else if(path != nullptr && strncmp(path, "../", 3) == 0){
        return true;
    }
    else {
        return false;
    }
}

bool is_absolute_path(const char* path) {
    if (path == nullptr) return false;

    // Case 1: Drive letter path (e.g., C:\ or D:/)
    if (std::isalpha(path[0]) && path[1] == ':' &&
        (path[2] == '\\' || path[2] == '/')) {
        return true;
    }

    // Case 2: UNC path (e.g., \\Server\Share)
    if (path[0] == '\\' && path[1] == '\\') {
        return true;
    }

    return false;
}

void init(char* argv[]) {
    if (argv[2] == NULL) {
        std::cout << "Please provide the path where the repository should be created\n";
        return;
    }

    const char* path = argv[2];

    std::string pathstr = path;
    pathstr += "/.minigit";
    std::string pathstrObjects = pathstr + "/objects";
    std::string headFilePath = pathstr + "/HEAD";
    std::string indexFilePath = pathstr + "/index";

    if (!is_relative_path(path) && !is_absolute_path(path)) {
        std::cout << "Please provide a valid Windows path\n";
        return;
    }

    if (_mkdir(pathstr.c_str()) == 0) {
        std::cout << ".minigit directory created successfully\n";
    } else {
        std::perror("mkdir failed");
    }

    if (_mkdir(pathstrObjects.c_str()) == 0) {
        std::cout << "Object directory created successfully\n";
    } else {
        std::perror("mkdir failed");
    }
    

    if(std::filesystem::exists(headFilePath)){
        std::cout << "HEAD file already exists";
    }
    else{
        std::ofstream headFile (headFilePath);
        headFile << "This is the head file\n" << std::endl;
    }

    if(std::filesystem::exists(indexFilePath)){
        std::cout << "index file already exists";
    }
    else{
        std::ofstream headFile (indexFilePath);
        headFile << "This is the index file\n" << std::endl;
    }

}