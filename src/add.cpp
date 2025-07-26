#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>

#include "util.hpp"
#include "add.hpp"

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

//probably needs refactoring, the code at the end of directory and file if statement is repeated
blob addToObjectsFolder(const std::filesystem::path& filePath, const std::filesystem::path& objectFolderPath){

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

        //recursively call addToObjects to build tree of blobs and trees
        std::vector<blob> listOfBlobs;
        for(const auto& entry: folderContents){
            std::cout << "file: " << entry; 
            listOfBlobs.emplace_back(addToObjectsFolder(entry, objectFolderPath));
        }

        //for each blob/tree add it to our tree content, this will be written to objects folder later
        std::stringstream treeFileContents;
        for(const blob& currentBlob : listOfBlobs){
            treeFileContents << currentBlob.fileTypeStr << " " << currentBlob.hash << " " << currentBlob.fileName << std::endl;
        }

        //add the tree header and the list of all blobs and trees after
        std::stringstream treeFile;
        treeFile << "tree " << treeFileContents.str().size() << '\0' << treeFileContents.str();

        //hash it and store it, also returns itself to build the tree back up for the recursive call
        std::string hashedObject = hashObject(treeFile.str());
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
        //return the blob if it already exists (deduplication)
        else{
            return blob(hashedObject, filePath.filename().string(), 
            std::filesystem::is_directory(filePath) ? FileType::TREE : FileType::BLOB);
        }

    }
    //serialize -> hash -> save -> return itself
    else{
        std::string serializedContent = serializeFile(filePath);
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
