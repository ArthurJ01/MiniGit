#include <filesystem>

#include "sha1.hpp"
#include "util.hpp"

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

void writeToObjectsFolder(const std::string& contents, const std::string& hash, const std::filesystem::path& repositoryRoot){
        
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

std::filesystem::path getCurrentBranchPath(const std::filesystem::path& repositoryRoot){
    std::filesystem::path headFilePath = repositoryRoot / ".minigit" / "HEAD";
    std::string currentBranchPathStr;
    std::filesystem::path currentBranchPath;
    std::ifstream headFile(headFilePath);
    if(!headFile){
        std::cerr << "could not open HEAD file";
        return currentBranchPath;
    }
    else{
        std::getline(headFile, currentBranchPathStr);
        currentBranchPath = currentBranchPathStr;
        currentBranchPath = repositoryRoot / ".minigit" / currentBranchPath;
        return currentBranchPath;
    }
}

std::string getParentCommitHash(const std::filesystem::path& repositoryRoot){

    std::filesystem::path headFilePath = repositoryRoot / ".minigit" / "HEAD";
    std::ifstream headFile(headFilePath);
    if(!headFile){
        std::cerr << "could not open HEAD file";
        return "";
    }

    std::stringstream buffer;
    buffer << headFile.rdbuf();
    std::string headContent = buffer.str();
    
    if (headContent.rfind("ref: ", 0) == 0) {
        std::string refPathStr = headContent.substr(5);
        std::filesystem::path refPath = repositoryRoot / ".minigit" / refPathStr;

        std::ifstream branchFile(refPath);
        if (!branchFile) {
            return "";
        }

        std::string commitHash;
        std::getline(branchFile, commitHash);
        return commitHash;
    }
    else{
        return headContent;
    }
}