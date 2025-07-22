#include <iostream>
#include <fstream>
#include <filesystem>
#include <string.h>
#include <direct.h>

#include "init.hpp"

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
    
    //check if HEAD file exists
    if(std::filesystem::exists(headFilePath)){
        std::cout << "HEAD file already exists, do you wish to overwrite?\ny/n \n";
        char response;
        std::cin >> response;

        //wishes to overwrite
        if (response == 'y' || response == 'Y') {
            std::ofstream headFile(headFilePath, std::ios::trunc);
            if (!headFile) {
                std::cerr << "Failed to open HEAD file for writing\n";
            } else {
                headFile << "This is the HEAD file\n";
            }
        //don't overwrite
        } else {
            std::cout << "File not overwritten.\n";
        }
    //file doesn't exist    
    } else {
        std::ofstream headFile(headFilePath);
        if (!headFile) {
            std::cerr << "Failed to create HEAD file\n";
        } else {
            headFile << "This is the HEAD file\n";
        }
    }


    //check if indexFile exists
    if(std::filesystem::exists(indexFilePath)){
        std::cout << "index file already exists, do you wish to overwrite?\ny/n \n";
        char response;
        std::cin >> response;

        //wishes to overwrite
        if (response == 'y' || response == 'Y') {
            std::ofstream indexFile(indexFilePath, std::ios::trunc);
            if (!indexFile) {
                std::cerr << "Failed to open index file for writing\n";
            } else {
                indexFile << "This is the index file\n";
            }
        //don't overwrite
        } else {
            std::cout << "File not overwritten.\n";
        }
    //file doesn't exist    
    } else {
        std::ofstream indexFile(indexFilePath);
        if (!indexFile) {
            std::cerr << "Failed to create index file\n";
        } else {
            indexFile << "This is the index file\n";
        }
    }

}