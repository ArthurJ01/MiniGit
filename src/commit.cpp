#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <vector>

#include "util.hpp"
#include "commit.hpp"

void commit (char* argv[]){

    //argv[2] returns true when not null
    std::string commitMessage = argv[2] ? argv[2] : " ";

    std::filesystem::path repositoryRoot = findRepositoryRoot(std::filesystem::current_path());
    std::filesystem::path indexFilePath = repositoryRoot / ".minigit" / "index";
    std::ifstream indexFileRead(indexFilePath);
    if(!indexFileRead){
        std::cout << "couldnt open Index file for the commit";
    }
    std::stringstream indexFileContents;
    indexFileContents << indexFileRead.rdbuf();
    indexFileRead.close();

    std::filesystem::path currentBranchPath = getCurrentBranchPath(repositoryRoot);

    //load index into hashmap for lookups, <pathString, hash>
    std::unordered_map<std::string, std::string> indexMap = loadIndex(repositoryRoot / ".minigit" / "index");
    blob commit = createCommitTree(repositoryRoot, repositoryRoot, indexMap);
    
    std::string parentCommitHash = getParentCommitHash(currentBranchPath);

    std::stringstream commitContents;
    commitContents << "tree " << commit.hash << "\n";
    if (!parentCommitHash.empty()) {
        commitContents << "parent " << parentCommitHash << "\n";
    }
    commitContents << "author username" << "\n";
    commitContents << "committer username" << "\n";

    commitContents << "\n" << commitMessage << "\n";
    std::string hash = hashObject(commitContents.str());
    writeToObjectsFolder(commitContents.str(), hash, repositoryRoot);
    std::ofstream branchFile(currentBranchPath, std::ios::trunc);
    if(!branchFile){
        std::cerr << "could not open head File";
        return;
    }
    else{
        branchFile << hash;
    }
    branchFile.close();
    //reset index file
    std::ofstream indexFileWrite(indexFilePath, std::ios::trunc);
    if(!indexFileWrite){
        std::cerr << "Failed to open index file";
    }
    indexFileWrite.close();
}
std::filesystem::path getCurrentBranchPath(const std::filesystem::path& repositoryRoot){
    std::filesystem::path headFilePath = repositoryRoot / ".minigit" / "HEAD";
    std::string currentBranchPathStr;
    std::filesystem::path currentBranchPath;
    std::ifstream headFile(headFilePath);
    if(!headFile){
        std::cerr << "could not open HEAD file";
        return currentBranchPath;
    }
    else{
        std::getline(headFile, currentBranchPathStr);
        currentBranchPath = currentBranchPathStr;
        currentBranchPath = repositoryRoot / ".minigit" / currentBranchPath;
        return currentBranchPath;
    }
}
std::string getParentCommitHash(const std::filesystem::path& headFilePath){
    std::ifstream headFile(headFilePath);
    if (!headFile) {
        return "";
    }

    std::string commitHash;
    std::getline(headFile, commitHash);

    if (commitHash.empty()) {
        return "";
    }

    return commitHash;
}

std::unordered_map<std::string, std::string> loadIndex(const std::filesystem::path& indexPath) {
    std::unordered_map<std::string, std::string> indexMap;
    std::ifstream indexFile(indexPath);
    if (!indexFile.is_open()) {
        std::cerr << "Could not open index file\n";
        return indexMap;
    }
    std::string line;
    while (std::getline(indexFile, line)) {
        auto commaPos = line.find(',');
        if (commaPos == std::string::npos) continue;
        std::string filePathStr = line.substr(0, commaPos);
        std::string hash = line.substr(commaPos + 1);
        indexMap[filePathStr] = hash;
    }
    return indexMap;
}

blob createCommitTree(std::filesystem::path filePath, std::filesystem::path repositoryRoot, const std::unordered_map<std::string, std::string>& indexMap){

    if (!std::filesystem::is_directory(filePath)) {
        throw std::runtime_error("createCommitTree() called on a non-directory path: " + filePath.string());
    }

    //get a list of all files/folders in current folder
    std::vector<std::filesystem::path> folderContents;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(filePath)) {
            folderContents.emplace_back(entry.path());
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }


    std::vector<blob> listOfBlobs;

    for(const auto& entry: folderContents){
        if(std::filesystem::is_directory(entry)){
            //has no files/folders
            if(std::filesystem::directory_iterator(entry) == std::filesystem::directory_iterator()){
                continue;
            }
            else{
                blob possibleBlob = createCommitTree(entry, repositoryRoot, indexMap);
                if(possibleBlob.fileType == FileType::EMPTY){
                    continue;
                }
                listOfBlobs.emplace_back(possibleBlob);
            }   
        }
        else if(std::filesystem::is_regular_file(entry)){
            std::string relativePath = std::filesystem::relative(entry, repositoryRoot).string();
            //if not in indexmap
            if (indexMap.find(relativePath) == indexMap.end()) {
                continue;
            }
            else{
                listOfBlobs.emplace_back(indexMap.at(relativePath), entry.filename().string(), FileType::BLOB);
            }
        }
    }

    if(listOfBlobs.empty()){
        return blob (FileType::EMPTY);
    }

    std::stringstream treeFileContents;
    for(const blob& currentBlob : listOfBlobs){
        if(currentBlob.fileType == FileType::EMPTY){
            continue;
        }
        treeFileContents << currentBlob.fileMode << " " << currentBlob.fileName;
        treeFileContents.put('\0');
        treeFileContents << currentBlob.hash;
    }
    
    std::stringstream treeFile;
    treeFile << "tree " << treeFileContents.str().size() << '\0' << treeFileContents.str();
    std::string hash = hashObject(treeFile.str());
    writeToObjectsFolder(treeFile.str(), hash, repositoryRoot);
    blob currentBlob (hash, filePath.filename().string(), FileType::TREE);
    return currentBlob; 
}