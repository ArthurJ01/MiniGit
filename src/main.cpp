#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <filesystem>

#include "init.hpp"
#include "sha1.hpp"
#include "util.hpp"



void add(char* argv[]){
    if (argv[2] == NULL) {
        std::cout << "Usage: add <filepath> \n";
        return;
    }
    std::filesystem::path filePath = argv[2];
    std::filesystem::path repositoryRoot = findRepositoryRoot(filePath);
    std::filesystem::path objectFolderPath = repositoryRoot / ".minigit" /"objects";

    std::string hashedObject;

    //check contents of directory and do add to each file
    if(std::filesystem::is_directory(filePath)){
        std::string serializedContent = serializeFile(filePath, FileType::TREE);
    }
    //create file and done
    else{
        std::string serializedContent = serializeFile(filePath, FileType::BLOB);
        hashedObject = hashObject(serializedContent);
        
        std::filesystem::path objectPath = objectFolderPath / hashedObject;

        if(!std::filesystem::exists(objectPath)){
            std::ofstream file(objectPath, std::ios::binary);
            if (!file) {
                std::cerr << "Failed to create object file\n" << objectPath;
            } else {
                file << serializedContent;
            }
        }
    }
    //add to index
}

int main(int argc, char* argv[]){

    std::string command = argv[1];

    if(argc == 1){
        std::cout << "Please provide an argument \n";
    }
    else {
        if(command == "init"){
            std::cout << "initializing repository... \n";
            init(argv);
        }
        else if(command == "add"){
            std::cout << "adding file... \n";
            add(argv);
        }
        else if(command == "commit"){
            std::cout << "commiting files... \n";
        }
        else if(command == "log"){
            std::cout << "printing files... \n";
        } 
        else{
            std::cout << "invalid argument: " << command << std::endl;
        }
    }

    return 0;
}