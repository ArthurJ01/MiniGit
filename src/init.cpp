#include <iostream>
#include <fstream>
#include <filesystem>
#include <string.h>
#include <direct.h>

#include "init.hpp"

void init(char* argv[]) {

    std::filesystem::path path;
    if (argv[2] == NULL) {
        path = ".";  
    }
    else{
        path = argv[2];
    }
    
    std::filesystem::path repositoryRoot = path / ".minigit";
    std::filesystem::path objectsFolderPath = repositoryRoot / "objects";
    std::filesystem::path indexFilePath = repositoryRoot / "index";
    std::filesystem::path headsFolderPath = repositoryRoot / "refs" / "heads";
    std::filesystem::path masterFilePath = headsFolderPath / "master";

    try{
        std::filesystem::create_directory(repositoryRoot);
        std::filesystem::create_directory(objectsFolderPath);
        std::filesystem::create_directories(headsFolderPath);
        createOrOverwriteFile(masterFilePath);
        createOrOverwriteFile(indexFilePath);
        std::cout << "repository successfully initialised";
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << '\n';
    }
}

void createOrOverwriteFile(const std::filesystem::path& filePath) {
    if (std::filesystem::exists(filePath)) {
        std::cout << filePath.filename() << " already exists, do you wish to overwrite? (y/n): ";
        char response;
        std::cin >> response;

        if (response == 'y' || response == 'Y') {
            std::ofstream file(filePath, std::ios::trunc);
            if (!file) {
                std::cerr << "Failed to overwrite " << filePath.filename() << "\n";
            }
        } else {
            std::cout << filePath.filename() << " not overwritten.\n";
        }
    } else {
        std::ofstream file(filePath);
        if (!file) {
            std::cerr << "Failed to create " << filePath.filename() << "\n";
        }
    }
}