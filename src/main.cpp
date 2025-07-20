#include <iostream>
#include <string.h>
#include <direct.h>

bool is_relative_path(const char* path) {

    if(path != nullptr && strncmp(path, "./", 2) == 0){
        return true;
    }
    else if(path != nullptr && strncmp(path, "../", 3) == 0){
        return true;
    }
    else {
        return false;
    }
}

bool is_absolute_path(const char* path) {
    if (path == nullptr) return false;

    // Case 1: Drive letter path (e.g., C:\ or D:/)
    if (std::isalpha(path[0]) && path[1] == ':' &&
        (path[2] == '\\' || path[2] == '/')) {
        return true;
    }

    // Case 2: UNC path (e.g., \\Server\Share)
    if (path[0] == '\\' && path[1] == '\\') {
        return true;
    }

    return false;
}

void init(char* argv[]) {
    if (argv[2] == NULL) {
        std::cout << "Please provide the path where the repository should be created\n";
        return;
    }

    const char* path = argv[2];

    if (!is_relative_path(path) && !is_absolute_path(path)) {
        std::cout << "Please provide a valid Windows path\n";
        return;
    }
    if (_mkdir(path) == 0) {
        std::cout << "Directory created successfully\n";
    } else {
        std::perror("mkdir failed");
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
            std::cout << "adding file... \n";
        }
        else if(command == "commit"){
            std::cout << "commiting files... \n";
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