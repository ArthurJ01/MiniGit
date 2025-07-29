#include <iostream>

#include "init.hpp"
#include "add.hpp"

#include "util.hpp"

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