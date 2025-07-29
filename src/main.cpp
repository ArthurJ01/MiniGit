#include <iostream>

#include "init.hpp"
#include "add.hpp"

#include "util.hpp"
#include "unordered_map"

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

    //THIS IS NOT IT
    /*
        for each file in index
        get parent, store in hashmap, child as key, parent as value, go up one level and do same with the folder
        
        testing shit
        using Tree = std::unordered_map<std::string, std::vector<blob>>;
        Tree treeMap;
    */

    //THIS IS IT!!!:
    /*
        do as if you did add on the whole repo
        When you get to a file, check if its in the index (yes keep, no throw away)
        Don't use empty directories
    */

    //std::filesystem::path objectFolderPath = repositoryRoot / ".minigit" / "objects";
   // createCommitTree(repositoryRoot);

    
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

blob createCommitTree(std::filesystem::path filePath, std::filesystem::path repositoryRoot){

    if (!std::filesystem::is_directory(filePath)) {
        throw std::runtime_error("createCommitTree() called on a non-directory path: " + filePath.string());
    }
    //has no files/folders
    if(std::filesystem::directory_iterator(filePath) == std::filesystem::directory_iterator()){
       // return;
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

    //load index into hashmap for lookups
    std::unordered_map<std::string, std::string> indexMap = loadIndex(repositoryRoot / ".minigit" / "index");

    /*
        recursively call addToObjects to build tree of blobs and trees
        if it's a file, it will check if its in the index map and if yes return the blob directly
        This is done so that the version added in add is the one commited, not the one currently there
    */
    std::vector<blob> listOfBlobs;

    //do a dfs, add things to a map <path, string> string is what will go in the tree object

    for(const auto& entry: folderContents){
        if(std::filesystem::is_regular_file(entry)){
            std::string relativePath = std::filesystem::relative(entry, repositoryRoot).string();
            //if not in indexmap
            if (indexMap.find(relativePath) == indexMap.end()) {
                continue;
            }
            else{
                listOfBlobs.emplace_back(indexMap.at(relativePath), entry.filename().string(), FileType::BLOB);
                continue;
            }
        }
        listOfBlobs.emplace_back(createCommitTree(entry, repositoryRoot));
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
    //  blob currentBlob (filePath, treeFile.str(), hash);
    blob currentBlob (hash, filePath.filename().string(), FileType::TREE);
    return currentBlob; 

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