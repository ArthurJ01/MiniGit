#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <filesystem>

#include "init.hpp"
#include "sha1.hpp"

enum class FileType{BLOB, TREE};

struct FileData {
    FileType fileType;
    std::string content;

    FileData(const FileType& fileType, const std::string& content)
        : fileType(fileType), content(content) {}
};

std::string hashObject(const FileData& fileData){

    std::stringstream os;
    std::string fileType;
    if (fileData.fileType == FileType::BLOB){
        fileType = "blob ";
    }
    else if (fileData.fileType == FileType::TREE){
        fileType = "tree ";
    }

    os << fileType << fileData.content.size() << '\0';
    os << fileData.content;

    SHA1 sha1;
    sha1.update(os.str());
    return sha1.final();
}

std::string serializeFile(const std::filesystem::path& filePath){
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if(!file){
        throw std::runtime_error("Failed to open file: " + filePath.string());
    }
    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}

void add(char* argv[]){
    if (argv[2] == NULL) {
        std::cout << "Usage: add <filepath> \n";
        return;
    }
    std::filesystem::path filePath = argv[2];
    FileType fileType = FileType::BLOB;
    std::string serializedFile = serializeFile(filePath);
    FileData fileData (fileType, serializedFile);
    std::string blob = hashObject(fileData);
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