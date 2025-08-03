#include <filesystem>
#include <sstream>
#include <fstream>

#include "branch.hpp"
#include "util.hpp"
//for the createOrOverwriteFile(), maybe move it
#include "init.hpp"

void branch(char* argv[]){

    if(argv[2] == nullptr){
        std::cerr << "Usage: branch <branch-name>\n";
        return;
    }
    std::string branchName = argv[2];
    std::filesystem::path repositoryRoot = findRepositoryRoot(std::filesystem::current_path());
    std::filesystem::path currentBranchPath = getCurrentBranchPath(repositoryRoot);
    std::filesystem::path newBranchPath = repositoryRoot / ".minigit" / "refs" / "heads" / branchName;
    if(!createOrOverwriteFile(newBranchPath)) return;

    //copy current branch file contents
    std::ifstream currentBranchFile(currentBranchPath, std::ios::binary);
    if(!currentBranchFile){
        std::cerr << "failed to open " << currentBranchPath << "\n";
        return;
    }
    std::stringstream fileContents;
    fileContents << currentBranchFile.rdbuf();
    currentBranchFile.close();

    //paste the content into the new branch file
    std::ofstream newBranchFile(newBranchPath, std::ios::binary);
    if(!newBranchFile){
        std::cerr << "failed to open " << newBranchPath << "\n";
        return;
    }
    newBranchFile << fileContents.str();

    std::cout << "new branch: " << newBranchPath.filename() << " created.\n";
}