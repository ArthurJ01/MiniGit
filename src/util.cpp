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

//turn file into "blob (or tree) <size>\0" + file contents as a string
const std::string serializeFile(const std::filesystem::path& filePath){

    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if(!file){
        throw std::runtime_error("Serializer failed to open file: " + filePath.string());
    }
    std::ostringstream content;
    std::ostringstream serializedFile;
    std::ostringstream fileHeader;

    content << file.rdbuf();
    fileHeader << "blob " << content.str().size() << '\0';
    serializedFile << fileHeader.str() << content.str();
    return serializedFile.str();
}