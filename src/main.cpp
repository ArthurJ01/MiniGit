#include <iostream>

#include "init.hpp"
#include "add.hpp"
#include "commit.hpp"
#include "log.hpp"
#include "checkout.hpp"
#include "branch.hpp"

void rebase(char* argv[]){
    if(argv[2] == nullptr || argv[3] == nullptr){
        std::cout << "Usage: rebase <branch to rebase into> <branch to be merged>";
        return;
    }

    std::filesystem::path repositoryRoot = findRepositoryRoot(std::filesystem::current_path());
    std::filesystem::path branchHeads = repositoryRoot / ".minigit" / "refs" / "heads";
    std::vector<std::filesystem::path> listOfAllBranches;

    try {
        for (const auto& entry : std::filesystem::directory_iterator(branchHeads)) {
            listOfAllBranches.emplace_back(entry.path());
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    std::string rebaseTarget = argv[2];
    std::string rebaseSource = argv[3];
    bool validTargetBranch = false;
    bool validSourceBranch = false;
    for(const auto& entry : listOfAllBranches){
        if(entry.filename() == rebaseTarget){
            validTargetBranch = true;
        }
        else if(entry.filename() == rebaseSource){
            validSourceBranch = true;
        }
    }

    if(!validSourceBranch || !validTargetBranch){
        std::cout << "Invalid branch name(s)";
        return;
    }

    //branchNames are valid now, find fork point (common ancestor) now

    //from fork point, compare source and target branch, create a new tree from this. 
    
    //When done make new commit, then move on to next commit


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
        else if(command == "rebase"){
            rebase(argv);
        }
        else{
            std::cout << "invalid argument: " << command << std::endl;
        }
    }

    return 0;
}