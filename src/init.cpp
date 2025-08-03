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
    
    std::filesystem::path miniGitDir = path / ".minigit";
    std::filesystem::path objectsFolderPath = miniGitDir / "objects";
    std::filesystem::path indexFilePath = miniGitDir / "index";
    std::filesystem::path headsFolderPath = miniGitDir / "refs" / "heads";
    std::filesystem::path headFilePath = miniGitDir / "HEAD";
    std::filesystem::path masterFilePath = headsFolderPath / "master";

    try{
        std::filesystem::create_directory(miniGitDir);
        std::filesystem::create_directory(objectsFolderPath);
        std::filesystem::create_directories(headsFolderPath);

        createOrOverwriteFile(masterFilePath);
        createOrOverwriteFile(indexFilePath);
        createOrOverwriteFile(headFilePath);
        std::ofstream headFile(headFilePath);
        masterFilePath = std::filesystem::relative(masterFilePath, miniGitDir);
        headFile << masterFilePath.string();
        headFile.close();

        std::cout << "repository successfully initialised";

    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << '\n';
    }
}

bool createOrOverwriteFile(const std::filesystem::path& filePath) {
    if (std::filesystem::exists(filePath)) {
        std::cout << filePath.filename() << " already exists, do you wish to overwrite? (y/n): ";
        char response;
        std::cin >> response;

        if (response == 'y' || response == 'Y') {
            std::ofstream file(filePath, std::ios::trunc);
            if (!file) {
                std::cerr << "Failed to overwrite " << filePath.filename() << "\n";
                return false;
            }
        } else {
            std::cout << filePath.filename() << " not overwritten.\n";
            return false;
        }
    } else {
        std::ofstream file(filePath);
        if (!file) {
            std::cerr << "Failed to create " << filePath.filename() << "\n";
            return false;
        }
    }
    return true;
}