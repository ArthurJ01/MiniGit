#include <iostream>

#include "init.hpp"
#include "add.hpp"

#include "util.hpp"
#include <unordered_map>
#include <unordered_set>
#include <stack>
void commit (char* argv[]);
std::unordered_map<std::string, std::string> loadIndex(const std::filesystem::path& indexPath);
blob createCommitTree(std::filesystem::path filePath, std::filesystem::path repositoryRoot, const std::unordered_map<std::string, std::string>& indexMap);
void writeTree(const std::string& contents, const std::string& hash, const std::filesystem::path& repositoryRoot);

void commit (char* argv[]){
    std::filesystem::path repositoryRoot = findRepositoryRoot(std::filesystem::current_path());
    std::filesystem::path indexFile = repositoryRoot / ".minigit" /"index";
    std::ifstream file(indexFile);
    if(!file){
        std::cout << "couldnt open Index file for the commit";
    }
    std::stringstream indexFileContents;
    indexFileContents << file.rdbuf();
    std::cout << indexFileContents.str();
    
    //load index into hashmap for lookups, <pathString, hash>
    std::unordered_map<std::string, std::string> indexMap = loadIndex(repositoryRoot / ".minigit" / "index");
    blob commit = createCommitTree(repositoryRoot, repositoryRoot, indexMap);
    
    
}

std::unordered_map<std::string, std::string> loadIndex(const std::filesystem::path& indexPath) {
    std::unordered_map<std::string, std::string> indexMap;
    std::ifstream indexFile(indexPath);
    if (!indexFile.is_open()) {
        std::cerr << "Could not open index file\n";
        return indexMap;
    }
    std::string line;
    while (std::getline(indexFile, line)) {
        auto commaPos = line.find(',');
        if (commaPos == std::string::npos) continue;
        std::string filePathStr = line.substr(0, commaPos);
        std::string hash = line.substr(commaPos + 1);
        indexMap[filePathStr] = hash;
    }
    return indexMap;
}

blob createCommitTree(std::filesystem::path filePath, std::filesystem::path repositoryRoot, const std::unordered_map<std::string, std::string>& indexMap){

    if (!std::filesystem::is_directory(filePath)) {
        throw std::runtime_error("createCommitTree() called on a non-directory path: " + filePath.string());
    }

    //get a list of all files/folders in current folder
    std::vector<std::filesystem::path> folderContents;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(filePath)) {
            folderContents.emplace_back(entry.path());
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }


    std::vector<blob> listOfBlobs;

    for(const auto& entry: folderContents){
        if(std::filesystem::is_directory(entry)){
            //has no files/folders
            if(std::filesystem::directory_iterator(entry) == std::filesystem::directory_iterator()){
                continue;
            }
            else{
                blob possibleBlob = createCommitTree(entry, repositoryRoot, indexMap);
                if(possibleBlob.fileType == FileType::EMPTY){
                    continue;
                }
                listOfBlobs.emplace_back(possibleBlob);
            }   
        }
        else if(std::filesystem::is_regular_file(entry)){
            std::string relativePath = std::filesystem::relative(entry, repositoryRoot).string();
            //if not in indexmap
            if (indexMap.find(relativePath) == indexMap.end()) {
                continue;
            }
            else{
                std::cout << "found this file in index: " << relativePath << std::endl;
                listOfBlobs.emplace_back(indexMap.at(relativePath), entry.filename().string(), FileType::BLOB);
            }
        }
    }

    if(listOfBlobs.empty()){
        return blob (FileType::EMPTY);
    }

    std::stringstream treeFileContents;
    for(const blob& currentBlob : listOfBlobs){
        if(currentBlob.fileType == FileType::EMPTY){
            continue;
        }
        treeFileContents << currentBlob.fileMode << " " << currentBlob.fileName;
        treeFileContents.put('\0');
        treeFileContents << currentBlob.hash;
    }
    
    std::stringstream treeFile;
    treeFile << "tree " << treeFileContents.str().size() << '\0' << treeFileContents.str();
    std::string hash = hashObject(treeFile.str());
    writeTree(treeFile.str(), hash, repositoryRoot);
    blob currentBlob (hash, filePath.filename().string(), FileType::TREE);
    return currentBlob; 
}

void writeTree(const std::string& contents, const std::string& hash, const std::filesystem::path& repositoryRoot){
        
    std::filesystem::path objectFolderPath = repositoryRoot / ".minigit" /"objects";
    std::filesystem::path objectPath = objectFolderPath / hash;

    if(!std::filesystem::exists(objectPath)){
        std::ofstream file(objectPath, std::ios::binary);
        if (!file) {
            std::cerr << "Trying to write to" << objectPath;
            throw std::runtime_error("Failed creating file \n");
        } else {
            file << contents;
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
            add(argv);
        }
        else if(command == "commit"){
            commit(argv);
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