#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

#include "init.hpp"
#include "sha1.hpp"

struct FileBlob {
    std::string filename;
    std::string content;
};

std::string hashObject(const FileBlob& blob){

    std::stringstream os;
    os << "filename: " << blob.filename << "\n";
    os << "content: " << blob.content << "\n";

    SHA1 sha1;
    sha1.update(os.str());
    return sha1.final();
}

std::string serializeFile(const std::string& filePath){
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if(!file){
        throw std::runtime_error("Failed to open file: " + filePath);
    }
    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}

void add(char* argv[]){

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