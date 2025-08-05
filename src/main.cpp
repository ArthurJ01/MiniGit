#include <iostream>

#include "init.hpp"
#include "add.hpp"
#include "commit.hpp"
#include "log.hpp"
#include "branch.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

struct treeBlob {
    std::string hash;
    std::string fileName;
    std::string fileMode;
    std::filesystem::path fullPath;

    treeBlob(std::string hash, std::string fileName, std::string fileMode, std::filesystem::path fullPath)
        : hash(hash), fileName(fileName), fileMode(fileMode), fullPath(fullPath) {}
};

bool areThereUncommitedChanges(const std::filesystem::path& repositoryRoot){
    std::filesystem::path indexFilePath = repositoryRoot / ".minigit" / "index";
    std::ifstream indexFile(indexFilePath, std::ios::binary);

    std::stringstream indexFileContents;
    indexFileContents << indexFile.rdbuf();
    if(indexFileContents.str().empty()){
        return false;
    }
    else{
        return true;
    }
}

std::stack<treeBlob>& findAllFiles(std::stack<treeBlob>& listOfBlobs, std::filesystem::path treeObjectPath, std::filesystem::path realFilePath){

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

        //std::cout << "mode: " << fileMode << "\nfileName: " << fileName << "\nfileHash: " << fileHash << "\n---\n";
        //std::cout << "done with file: " << realFilePath;

        //next starting position is 40 bytes hex
        startingPos = nextNullBytePos + 41;
    }
    return listOfBlobs;
}

void checkout(char* argv[]){

    if(argv[2] == nullptr){
        std::cout << "Usage: checkout <branch-name>";
        return;
    }
    std::string branchName = argv[2];

    std::filesystem::path repositoryRoot = findRepositoryRoot(std::filesystem::current_path());
    std::filesystem::path headFilePath = repositoryRoot / ".minigit" / "HEAD";
    std::filesystem::path branchPath = repositoryRoot / ".minigit" / "refs" / "heads" / branchName;
    std::filesystem::path relativeBranchPath = std::filesystem::relative(branchPath, repositoryRoot / ".minigit");

    //warn if uncommited files exist, refuse to checkout
    bool uncommitedChanges = areThereUncommitedChanges(repositoryRoot);

    if(uncommitedChanges){
        std::cout << "there are uncommited changes, you will lose all uncommited changes if you checkout another commit/branch. Do you still wish to proceed: y/n \n";
        char answer;
        std::cin >> answer;

        if((answer == 'Y') | (answer == 'y')){
            std::cout << "checking out...";
        }
        else{
            std::cout << "cancelling checkout...";
            return;
        }
    }
    //if checkout on commit, store commit in detached HEAD (do this later)
    //if checkout on branch, change path in HEAD file
    std::ofstream headFile(headFilePath, std::ios::trunc);
    headFile << relativeBranchPath.string();
    headFile.close();
    
    //delete all content in the repo
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

    //add all content from commit in wanted branch

    //find contents of branchFile (in refs/heads/branchPath), this is a hash
    std::ifstream branchFile(branchPath, std::ios::binary);
    std::stringstream branchFileContents;
    branchFileContents << branchFile.rdbuf();
    branchFile.close();

    //use the hash found before to open the commit object, which contains a tree hash
    std::filesystem::path objectFolderPath = repositoryRoot / ".minigit" / "objects";
    std::filesystem::path branchCommitPath = objectFolderPath / branchFileContents.str();
    std::ifstream branchCommit(branchCommitPath, std::ios::binary);
    std::string branchTreeLine;
    std::getline(branchCommit, branchTreeLine);
    size_t hashStart = branchTreeLine.find(' ');
    std::string branchTreeHash = branchTreeLine.substr(hashStart + 1, 40);
    branchCommit.close();

    std::stack<treeBlob> listOfBlobs;
    //std::unordered_set<std::string> visitedTrees;

    //use the tree hash to find all files
    std::filesystem::path treeObjectPath = objectFolderPath / branchTreeHash;
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
            /*
            if(visitedTrees.find(currentBlob.hash) != visitedTrees.end()){
                continue;
            }

            visitedTrees.emplace(currentBlob.hash);
            */
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
            log();
        } 
        else if(command == "checkout"){
            checkout(argv);
        }
        else if(command == "branch"){
            branch(argv);
        }
        else{
            std::cout << "invalid argument: " << command << std::endl;
        }
    }

    return 0;
}