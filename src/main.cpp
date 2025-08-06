#include <iostream>

#include "init.hpp"
#include "add.hpp"
#include "commit.hpp"
#include "log.hpp"
#include "checkout.hpp"
#include "branch.hpp"



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