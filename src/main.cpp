#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <filesystem>

#include "init.hpp"
#include "sha1.hpp"

enum class FileType{BLOB, TREE};

std::string hashObject(const std::string& fileData){
    SHA1 sha1;
    sha1.update(fileData);
    return sha1.final();
}

const std::string serializeFile(const std::filesystem::path& filePath, const FileType& fileType){
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if(!file){
        throw std::runtime_error("Failed to open file: " + filePath.string());
    }
    std::ostringstream content;
    std::ostringstream serializedFile;
    std::ostringstream fileHeader;

    content << file.rdbuf();

    switch(fileType){
        case FileType::BLOB :
            fileHeader << "blob " << content.str().size() << '\0';
        break;

        case FileType::TREE :
            fileHeader << "tree " << content.str().size() << '\0';
        break;
    }
    serializedFile << fileHeader.str() << content.str();
    return serializedFile.str();
}

void add(char* argv[]){
    if (argv[2] == NULL) {
        std::cout << "Usage: add <filepath> \n";
        return;
    }
    std::filesystem::path filePath = argv[2];

    // TODO: this should be the objects folder path
    std::filesystem::path objectFolderPath = filePath;

    std::string hashedObject;

    if(std::filesystem::is_directory(filePath)){
        std::string serializedContent = serializeFile(filePath, FileType::TREE);
    }
    else{
        std::string serializedContent = serializeFile(filePath, FileType::BLOB);
        hashedObject = hashObject(serializedContent);
        
        std::filesystem::path objectPath = objectFolderPath / hashedObject;

        if(!std::filesystem::exists(objectPath)){
            std::ofstream file(objectPath, std::ios::binary);
            if (!file) {
                std::cerr << "Failed to create HEAD file\n";
            } else {
                file << serializedContent;
            }
        }
    }
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