#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <filesystem>

#include "init.hpp"
#include "sha1.hpp"

struct FileData {
    std::string fileName;
    std::string content;

    FileData(const std::string& fileName, const std::string& content)
        : fileName(fileName), content(content) {}
};

std::string hashObject(const FileData& blob){

    std::stringstream os;
    os << "filename: " << blob.fileName << "\n";
    os << "content: " << blob.content << "\n";

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
    std::string fileName = filePath.filename().string();
    std::string serializedFile = serializeFile(filePath);
    FileData fileData (fileName, serializedFile);
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