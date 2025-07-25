#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>

#include "init.hpp"
#include "sha1.hpp"
#include "util.hpp"

struct blob{
    std::string hash;
    std::string fileName;
    std::string fileTypeStr;

    blob(std::string hash, std::string fileName, FileType fileType) 
        :  hash(hash), fileName(fileName)
    {
        switch (fileType){

            case FileType::BLOB :
                fileTypeStr = "blob";
            break;
            case FileType::TREE :
                fileTypeStr = "tree";
            break;
        }

    }
};


//probably needs refactoring, the code at the end of directory and file if statement is repeated
blob addToObjectsFolder(const std::filesystem::path& filePath, const std::filesystem::path& objectFolderPath){

    std::vector<blob> listOfBlobs;

    if(std::filesystem::is_directory(filePath)){
        //get a list of all files/folders in current folder
        std::vector<std::filesystem::path> folderContents;
        try {
            for (const auto& entry : std::filesystem::directory_iterator(filePath)) {
                folderContents.emplace_back(entry.path());
            }
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }

        for(const auto& entry: folderContents){
            std::cout << "file: " << entry; 
            listOfBlobs.emplace_back(addToObjectsFolder(entry, objectFolderPath));
            //save type, hash, name
        }
        //do add (serialize + hash) on all of them

        std::stringstream treeFileContents;

        for(const blob& currentBlob : listOfBlobs){
            treeFileContents << currentBlob.fileTypeStr << " " << currentBlob.hash << " " << currentBlob.fileName << std::endl;
        }

        std::string hashedObject = hashObject(treeFileContents.str());
        std::filesystem::path objectPath = objectFolderPath / hashedObject;

        if(!std::filesystem::exists(objectPath)){
            std::ofstream file(objectPath, std::ios::binary);
            if (!file) {
                std::cerr << "Failed to create object file\n" << objectPath;
            } else {
                file << treeFileContents.str();
                blob currentBlob(hashedObject, filePath.filename().string(), FileType::TREE);
                return currentBlob;
            }
        }
        else{
            return blob(hashedObject, filePath.filename().string(), 
            std::filesystem::is_directory(filePath) ? FileType::TREE : FileType::BLOB);
        }

    }
    //create file and done
    else{
        std::string serializedContent = serializeFile(filePath, FileType::BLOB);
        std::string hashedObject = hashObject(serializedContent);
        
        std::filesystem::path objectPath = objectFolderPath / hashedObject;

        if(!std::filesystem::exists(objectPath)){
            std::ofstream file(objectPath, std::ios::binary);
            if (!file) {
                std::cerr << "Failed to create object file\n" << objectPath;
            } else {
                file << serializedContent;
                blob currentBlob(hashedObject, filePath.filename().string(), FileType::BLOB);
                return currentBlob;
            }
        }
        else{
            return blob(hashedObject, filePath.filename().string(), 
            std::filesystem::is_directory(filePath) ? FileType::TREE : FileType::BLOB);
        }
    }

    throw std::runtime_error("Failed to create object file \n");
}

void add(char* argv[]){
    if (argv[2] == NULL) {
        std::cout << "Usage: add <filepath> \n";
        return;
    }
    std::filesystem::path filePath = argv[2];
    std::filesystem::path repositoryRoot = findRepositoryRoot(filePath);
    std::filesystem::path objectFolderPath = repositoryRoot / ".minigit" /"objects";

    //add check to not add the .minigit folder
    addToObjectsFolder(filePath, objectFolderPath);
   
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