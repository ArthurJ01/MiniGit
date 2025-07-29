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
