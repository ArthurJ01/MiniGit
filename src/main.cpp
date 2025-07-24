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

std::filesystem::path findRepositoryRoot(std::filesystem::path start) {
    while (!start.empty()) {
        if (std::filesystem::exists(start / ".minigit")) {
            return start;
        }
        if(start == start.root_path()) break;
        start = start.parent_path();
    }
    throw std::runtime_error("Not inside a valid repository");
}

//turn file into "blob (or tree) <size>\0" + file contents as a string
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