#include <filesystem>
#include <sstream>
#include <fstream>
#include <stack>
#include <iostream>

#include "checkout.hpp"
#include "util.hpp"

void checkout(char* argv[]){

    std::filesystem::path repositoryRoot = findRepositoryRoot(std::filesystem::current_path());
    std::filesystem::path objectFolderPath = repositoryRoot / ".minigit" / "objects";

    //warns if uncommited files exist, cancel checkout on true
    if(areThereUncommitedChanges(repositoryRoot)) return;

    if(argv[2] == nullptr){
        std::cout << "Usage: checkout <branch-name>";
        return;
    }
    std::string checkoutTarget = argv[2];

    std::string commitHash = getCommitHash(repositoryRoot, checkoutTarget);
    if(commitHash == ""){
        std::cout << "provide a valid branch name or commit hash";
        return;
    }
    std::string treeHash = getTreeObjectHash(repositoryRoot, commitHash);
    deleteAllContentInRepository(repositoryRoot);

    std::stack<treeBlob> listOfBlobs;

    //use the tree hash to find all files
    std::filesystem::path treeObjectPath = objectFolderPath / treeHash;
    findAllFiles(listOfBlobs, treeObjectPath, repositoryRoot);

    while (!listOfBlobs.empty()){
        treeBlob currentBlob = listOfBlobs.top();
        listOfBlobs.pop();

        //normal file
        if(currentBlob.fileMode == "100644"){
            std::ofstream file(currentBlob.fullPath);
            if (!file) {
                std::cerr << "Failed to create " << currentBlob.fileName << "\n";
            }
            std::ifstream fileContents(objectFolderPath / currentBlob.hash);
            std::stringstream contents;
            contents << fileContents.rdbuf();
            std::string buffer = contents.str();
            size_t nullBytePos = buffer.find('\0');

            if (nullBytePos != std::string::npos) {
                std::string contentsWithoutHeader = buffer.substr(nullBytePos + 1);
                file << contentsWithoutHeader;
            } else {
                // no null byte found, dump what we have?
                file << contents.str();
            }  
        }
        //directory
        else if(currentBlob.fileMode == "040000"){
            std::filesystem::create_directory(currentBlob.fullPath);
            findAllFiles(listOfBlobs, objectFolderPath / currentBlob.hash, currentBlob.fullPath);
        }
        else{
            //filemode not implemented
            //100755 executable
            //120000 symbolic link
        }
    }
}

bool areThereUncommitedChanges(const std::filesystem::path& repositoryRoot){
    std::filesystem::path indexFilePath = repositoryRoot / ".minigit" / "index";
    std::ifstream indexFile(indexFilePath, std::ios::binary);

    std::stringstream indexFileContents;
    indexFileContents << indexFile.rdbuf();

    bool uncommitedChanges;

    if(indexFileContents.str().empty()){
        uncommitedChanges = false;
    }
    else{
        uncommitedChanges = true;
    }

    if(uncommitedChanges){
        std::cout << "there are uncommited changes, you will lose all uncommited changes if you checkout another commit/branch. Do you still wish to proceed: y/n \n";
        char answer;
        std::cin >> answer;

        if((answer == 'Y') | (answer == 'y')){
            std::cout << "checking out...";
            return false;
        }
        else{
            std::cout << "cancelling checkout...";
            return true;
        }
    }
    return false;
}

std::stack<treeBlob>& findAllFiles(std::stack<treeBlob>& listOfBlobs, const std::filesystem::path& treeObjectPath, const std::filesystem::path& realFilePath){

    std::ifstream branchTree(treeObjectPath, std::ios::binary);
    std::stringstream contents;
    contents << branchTree.rdbuf();
    std::string line = contents.str();
    size_t startingPos = line.find('\0') + 1;
    if(startingPos == std::string::npos){
        std::cout << "no content in branch \n";
        return listOfBlobs;
    }

    while (startingPos < line.size()){
        size_t spacePos = line.find(' ', startingPos);
        size_t nextNullBytePos = line.find('\0', spacePos);
        std::string fileMode = line.substr(startingPos, spacePos - (startingPos));
        std::string fileName = line.substr(spacePos + 1, nextNullBytePos - (spacePos + 1));
        //sha1 hash stored as hex 40 bytes long
        std::string fileHash = line.substr(nextNullBytePos + 1, 40); 
        listOfBlobs.emplace(fileHash, fileName, fileMode, realFilePath / fileName);
        //next starting position is 40 bytes hex
        startingPos = nextNullBytePos + 41;
    }
    return listOfBlobs;
}

std::string getCommitHash(const std::filesystem::path& repositoryRoot, const std::string& checkoutTarget){
    std::filesystem::path objectFolderPath = repositoryRoot / ".minigit" / "objects";
    std::filesystem::path headsFolderPath = repositoryRoot / ".minigit" / "refs" / "heads";
    CheckoutTargetType checkoutTargetType;

    for (const auto& entry : std::filesystem::directory_iterator(objectFolderPath)) {
        if(entry.path().filename() == checkoutTarget){
            checkoutTargetType = CheckoutTargetType::COMMIT;
        }
    }
    for (const auto& entry : std::filesystem::directory_iterator(headsFolderPath)) {
        if(entry.path().filename() == checkoutTarget){
            checkoutTargetType = CheckoutTargetType::BRANCH;
        }
    }

    std::filesystem::path headFilePath = repositoryRoot / ".minigit" / "HEAD";
    std::ofstream headFile(headFilePath, std::ios::trunc);
    std::string commitHash;

    switch(checkoutTargetType){
        case CheckoutTargetType::COMMIT : { 
            std::cout << "checking out a commit, further commits will be detached and not saved to a branch \n";
            headFile << checkoutTarget;
            headFile.close();
            commitHash = checkoutTarget;
            break;
        }

        case CheckoutTargetType::BRANCH : {   
            
            std::filesystem::path checkoutTargetPath = headsFolderPath / checkoutTarget;
            headFile << "ref: " << std::filesystem::relative(checkoutTargetPath, repositoryRoot / ".minigit").string();
            headFile.close();

            //find contents of branchFile (in refs/heads/checkoutTargetPath), this is a hash to a commit object
            std::ifstream branchFile(checkoutTargetPath, std::ios::binary);
            std::stringstream branchFileContents;
            branchFileContents << branchFile.rdbuf();
            commitHash = branchFileContents.str();
            branchFile.close();
    
            break;
        }
        default : {
            commitHash = "";
            break;
        }     
    }
    return commitHash;
}

void deleteAllContentInRepository(const std::filesystem::path& repositoryRoot){
    try {
        for (const auto& entry : std::filesystem::directory_iterator(repositoryRoot)) {
            if(entry.path().filename() == ".minigit"){
                continue;
            }
            else{
                std::error_code ec;
                std::filesystem::remove_all(entry.path());
                if (ec) {
                    std::cerr << "Failed to remove: " << entry.path() << " | Error: " << ec.message() << std::endl;
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

std::string getTreeObjectHash(const std::filesystem::path& repositoryRoot, const std::string& commitHash){
    std::filesystem::path objectFolderPath = repositoryRoot / ".minigit" / "objects";
    std::filesystem::path commitPath = objectFolderPath / commitHash;
    std::ifstream commit(commitPath, std::ios::binary);
    std::string branchTreeLine;
    std::getline(commit, branchTreeLine);
    size_t hashStart = branchTreeLine.find(' ');
    std::string treeObjectHash = branchTreeLine.substr(hashStart + 1, 40);
    commit.close();

    return treeObjectHash;
}

