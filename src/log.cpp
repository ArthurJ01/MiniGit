#include <filesystem>
#include <string>
#include <fstream>

#include "log.hpp"
#include "util.hpp"

void log(){
    std::filesystem::path repositoryRoot = findRepositoryRoot(std::filesystem::current_path());
    std::string parentCommitHash = getParentCommitHash(repositoryRoot);
    std::filesystem::path nextHeadFilePath;

    std::filesystem::path headFilePath = repositoryRoot / ".minigit" / "objects" / parentCommitHash;

    std::ifstream headFile(headFilePath);
    std::stringstream log;

    bool parentFound = false; 
    do{
        log << "----------------------------------------\n";
        parentFound = false;
        std::string line;
        while (std::getline(headFile, line)) {
            size_t spacePos = line.find(' ');
            if (spacePos != std::string::npos) {        
                std::string title = line.substr(0, spacePos);
                std::string content = line.substr(spacePos + 1);
                if(title == "parent"){
                    nextHeadFilePath = repositoryRoot / ".minigit" / "objects" / content;
                    parentFound = true;
                }
            }
            log << line << "\n";
        }
        headFile.close();
        if(parentFound){
            headFile.open(nextHeadFilePath);
        }

    } while (parentFound);
    std::cout << log.str();
}
