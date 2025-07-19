#include <iostream>

int main(int argc, char* argv[]){

    if(argc != 2){
        std::cout << "invalid number of arguments \n";
    }

    std::string command = argv[1];

    if(argc == 2){
        if(command == "init"){
            std::cout << "initializing repository... \n";
        }
        else if(command == "add"){
            std::cout << "adding file... \n";
        }
        else if(command == "commit"){
            std::cout << "commiting files... \n";
        }
        else if(command == "log"){
            std::cout << "printing files... \n";
        }
    }

    return 0;
}