#include <iostream>

#include "init.hpp"
#include "add.hpp"
#include "commit.hpp"
#include "log.hpp"

void checkout(char* argv[]){
    
}

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