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
    addToObjectsFolder(filePath, repositoryRoot);

}

void addToIndexFile(const std::filesystem::path& filePath, const std::string& hash, const std::filesystem::path& repositoryRoot){
    std::filesystem::path indexPath = repositoryRoot / ".minigit" / "index";
    std::stringstream indexEntry;
    std::filesystem::path relativePath;

    try {
        relativePath = std::filesystem::relative(filePath, repositoryRoot);
        indexEntry << relativePath.string() << "," << hash;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error computing relative path: " << e.what() << std::endl;
        return;
    }

    std::ifstream file(indexPath, std::ios::app | std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open index file for reading\n" << indexPath;
        return;
    }

    std::string line;
    std::vector<std::string> updatedLines;

    //each line of file gets put into line string
    while (std::getline(file, line)) {
        //find the position of the comma
        size_t commaPos = line.find(',');
        //if there is a comma on this line
        if (commaPos != std::string::npos) {
            //take all chars from pos 0 to commaposition
            std::string entryPath = line.substr(0, commaPos);
            std::string entryHash = line.substr(commaPos + 1);
            //if we already have an entry for this
            if (relativePath == entryPath && hash == entryHash) {
                return;
            }
            else if (entryPath == relativePath.string() && entryHash != hash){
                continue;
            }
        }

        std::stringstream newLine;
        newLine << line;
        updatedLines.emplace_back(newLine.str());
    }

    file.close();

    std::ofstream ofile(indexPath, std::ios::trunc | std::ios::binary);
    if (!ofile) {
        std::cerr << "Failed to open index file for writing\n" << indexPath;
        return;
    }
    for (const auto& updatedLine : updatedLines) {
        ofile << updatedLine << "\n";   
        std::cout << "writing file to index: 1" << updatedLine << std::endl;
    } 
    ofile << indexEntry.str() << "\n";
    std::cout << "writing file to index: 2" << indexEntry.str() << std::endl;
}

//rewrite this to not add folders (done at commit time)
blob addToObjectsFolder(const std::filesystem::path& filePath, const std::filesystem::path& repositoryRoot){
    
    if (filePath.filename() == ".minigit") return blob("", "", FileType::TREE);
    std::filesystem::path objectFolderPath = repositoryRoot / ".minigit" /"objects";
    
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
            listOfBlobs.emplace_back(addToObjectsFolder(entry, repositoryRoot));
        }

        std::stringstream treeFileContents;
        for(const blob& currentBlob : listOfBlobs){
            treeFileContents << currentBlob.fileMode << " " << currentBlob.fileName;
            treeFileContents.put('\0');
            treeFileContents << currentBlob.hash;
        }
        
        std::stringstream treeFile;
        treeFile << "tree " << treeFileContents.str().size() << '\0' << treeFileContents.str();
        std::string hash = hashObject(treeFile.str());

        return writeBlob(filePath, treeFile.str(), hash, repositoryRoot); 

    }
    //serialize -> hash -> save -> return itself
    else{
        std::string serializedContent = serializeFile(filePath);
        std::string hash = hashObject(serializedContent);
        blob currentBlob = writeBlob(filePath, serializedContent, hash, repositoryRoot);
        addToIndexFile(filePath, hash, repositoryRoot);
        return currentBlob; 
    }

    throw std::runtime_error("Failed to create object file in addToObjectsFolder \n");
}


blob writeBlob(const std::filesystem::path& filePath, const std::string& contents, const std::string& hash, const std::filesystem::path& repositoryRoot){
        
    std::filesystem::path objectFolderPath = repositoryRoot / ".minigit" /"objects";
    std::filesystem::path objectPath = objectFolderPath / hash;
    FileType fileType;

    if(std::filesystem::is_directory(filePath)){
        fileType = FileType::TREE;
    }
    else{
        fileType = FileType::BLOB;
    }

    if(!std::filesystem::exists(objectPath)){
        std::ofstream file(objectPath, std::ios::binary);
        if (!file) {
            std::cerr << "Trying to write to" << objectPath;
            throw std::runtime_error("Failed creating file \n");
        } else {
            file << contents;
        }
    }
    blob currentBlob(hash, filePath.filename().string(), fileType);
    return currentBlob;
}