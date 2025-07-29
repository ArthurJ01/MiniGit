#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>

#include "util.hpp"
#include "add.hpp"

#include <stack>
#include <unordered_set>

void add(char* argv[]){
    if (argv[2] == NULL) {
        std::cout << "Usage: add <filepath> \n";
        return;
    }
    std::filesystem::path filePath = argv[2];
    std::filesystem::path repositoryRoot = findRepositoryRoot(filePath);
    std::stack<std::filesystem::path> openList;
    std::unordered_set<std::filesystem::path> visited;

    visited.insert(filePath);
    openList.push(filePath);

    if (!std::filesystem::exists(filePath)) {
        std::cerr << "Error: Path does not exist.\n";
        return;
    }

    if (filePath.filename() == ".minigit") {
        std::cerr << "Error: Cannot add .minigit (internal repository folder).\n";
        return;
    }

    while(!openList.empty()){
        std::filesystem::path currentPath = openList.top();
        openList.pop();

        if(std::filesystem::is_directory(currentPath)){
            try {
                for (const auto& entry : std::filesystem::directory_iterator(currentPath)) {
                    if (entry.path().filename() == ".minigit") continue;
                    if (visited.find(entry) != visited.end()) continue;
                    visited.insert(entry);
                    openList.push(entry);
                }
            } catch (const std::filesystem::filesystem_error& e) {
                std::cerr << "Error: " << e.what() << std::endl;
            }
        }
        else if (std::filesystem::is_regular_file(currentPath)){
            
            std::ifstream file(currentPath, std::ios::in | std::ios::binary);
            if(!file){
                throw std::runtime_error("Serializer failed to open file: " + filePath.string());
            }
            std::ostringstream content;
            content << file.rdbuf();
            std::string serializedContent = content.str();
            std::string hash = hashObject(serializedContent);
            writeBlob(serializedContent, hash, repositoryRoot);
            addToIndexFile(currentPath, hash, repositoryRoot);
        }
    }

}


void writeBlob(const std::string& contents, const std::string& hash, const std::filesystem::path& repositoryRoot){
        
    std::filesystem::path objectFolderPath = repositoryRoot / ".minigit" /"objects";
    std::filesystem::path objectPath = objectFolderPath / hash;

    if(!std::filesystem::exists(objectPath)){
        std::ofstream file(objectPath, std::ios::binary);
        if (!file) {
            std::cerr << "Trying to write to" << objectPath;
            throw std::runtime_error("Failed creating file \n");
        } else {
            std::stringstream blobContents;
            blobContents << "blob " << contents.size() << '\0' << contents;
            file << blobContents.str();
        }
    }
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

    std::ifstream file(indexPath, std::ios::binary);
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
            if (relativePath.string() == entryPath && hash == entryHash) {
                return;
            }
            else if (relativePath.string() == entryPath && hash != entryHash){
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
    } 
    ofile << indexEntry.str() << "\n";
}