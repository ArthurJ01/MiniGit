#include <iostream>

#include "init.hpp"
#include "add.hpp"
#include "commit.hpp"
#include "log.hpp"
#include "checkout.hpp"
#include "branch.hpp"

void merge(char* argv[]){
    if(argv[2] == nullptr || argv[3] == nullptr){
        std::cout << "Usage: merge <branch to merge into> <branch to be merged>";
        return;
    }

    std::filesystem::path repositoryRoot = findRepositoryRoot(std::filesystem::current_path());
    std::filesystem::path branchHeads = repositoryRoot / ".minigit" / "refs" / "heads";
    std::vector<std::filesystem::path> listOfAllBranches;

    
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
        else if(command == "merge"){
            merge(argv);
        }
        else{
            std::cout << "invalid argument: " << command << std::endl;
        }
    }

    return 0;
}