#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <vector>

#include "util.hpp"
#include "commit.hpp"
#include "checkout.hpp"


//needs optimisation and, needs to add untracked files that were in last commit that haven't changed.
void commit (char* argv[]){

    //argv[2] returns true when not null
    std::string commitMessage = argv[2] ? argv[2] : " ";

    std::filesystem::path repositoryRoot = findRepositoryRoot(std::filesystem::current_path());
    std::filesystem::path indexFilePath = repositoryRoot / ".minigit" / "index";

    //load index into hashmap for lookups, <pathString, hash>
    std::string parentCommitHash = getParentCommitHash(repositoryRoot);
    std::unordered_map<std::filesystem::path, std::string> indexMap = loadIndex(indexFilePath);
    std::unordered_map<std::filesystem::path, std::string> lastCommitMap = loadLastCommit(repositoryRoot, parentCommitHash);
    blob commit = createCommitTree(repositoryRoot, repositoryRoot, indexMap, lastCommitMap);
    


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
    updateHeadFile(repositoryRoot, hash);

    //reset index file
    std::ofstream indexFileWrite(indexFilePath, std::ios::trunc);
    if(!indexFileWrite){
        std::cerr << "Failed to open index file";
    }
    indexFileWrite.close();
}


std::unordered_map<std::filesystem::path, std::string> loadLastCommit(const std::filesystem::path& repositoryRoot, const std::string& parentCommitHash){

    std::unordered_map<std::filesystem::path, std::string> commitMap;
    if(parentCommitHash == ""){ 
        return commitMap;
    }

    std::filesystem::path objectFolderPath = repositoryRoot / ".minigit" / "objects";
    std::filesystem::path parentCommitPath = objectFolderPath / parentCommitHash;
    std::ifstream parentCommit(parentCommitPath, std::ios::binary);
    std::string line;
    std::string lastCommitHash = "";

    while (std::getline(parentCommit, line)) {
        size_t spacePos = line.find(' ');
        if (spacePos != std::string::npos) {        
            std::string title = line.substr(0, spacePos);
            std::string content = line.substr(spacePos + 1);
            if(title == "tree"){
                lastCommitHash = content;
            }
        }
    }
    
    parentCommit.close();
    std::filesystem::path treePath = objectFolderPath / lastCommitHash;

    std::stack<treeBlob> listOfBlobs;
    findAllFiles(listOfBlobs, treePath, repositoryRoot);

    while (!listOfBlobs.empty()){
        treeBlob currentBlob = listOfBlobs.top();
        listOfBlobs.pop();

        //normal file
        if(currentBlob.fileMode == "100644"){
            std::filesystem::path relativePath = std::filesystem::relative(currentBlob.fullPath, repositoryRoot);
            commitMap[relativePath] = currentBlob.hash;
        }
        //directory
        else if(currentBlob.fileMode == "040000"){
            findAllFiles(listOfBlobs, objectFolderPath / currentBlob.hash, currentBlob.fullPath);
        }
        else{
            //filemode not implemented
            //100755 executable
            //120000 symbolic link
        }
    }

    return commitMap;
    
}

std::unordered_map<std::filesystem::path, std::string> loadIndex(const std::filesystem::path& indexPath) {
    std::unordered_map<std::filesystem::path, std::string> indexMap;
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
        std::filesystem::path filePath = filePathStr;
        std::string hash = line.substr(commaPos + 1);
        indexMap[filePath] = hash;
    }
    return indexMap;
}

blob createCommitTree(std::filesystem::path filePath, std::filesystem::path repositoryRoot, const std::unordered_map<std::filesystem::path, std::string>& indexMap, const std::unordered_map<std::filesystem::path, std::string>& lastCommitMap){

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
                blob possibleBlob = createCommitTree(entry, repositoryRoot, indexMap, lastCommitMap);
                if(possibleBlob.fileType == FileType::EMPTY){
                    continue;
                }
                listOfBlobs.emplace_back(possibleBlob);
            }   
        }
        else if(std::filesystem::is_regular_file(entry)){
            std::filesystem::path relativePath = std::filesystem::relative(entry, repositoryRoot);
            if (indexMap.find(relativePath) != indexMap.end()) {
                listOfBlobs.emplace_back(indexMap.at(relativePath), entry.filename().string(), FileType::BLOB);
                std::cout << "commiting file: " << entry.filename() <<"\n";
            } else if (lastCommitMap.find(relativePath) != lastCommitMap.end()) {
                listOfBlobs.emplace_back(lastCommitMap.at(relativePath), entry.filename().string(), FileType::BLOB);
                std::cout << "commiting file: " << entry.filename() <<"\n";
            } else {
                continue;
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

void updateHeadFile(const std::filesystem::path& repositoryRoot, const std::string& hash){

    std::filesystem::path headFilePath = repositoryRoot / ".minigit" / "HEAD";
    std::ifstream headFile(headFilePath, std::ios::binary);
    if(!headFile){
        std::cerr << "could not open HEAD file";
    }

    std::stringstream buffer;
    buffer << headFile.rdbuf();
    std::string headContent = buffer.str();
    
    if (headContent.rfind("ref: ", 0) == 0) {
        std::string refPathStr = headContent.substr(5);
        std::filesystem::path refPath = repositoryRoot / ".minigit" / refPathStr;

        std::ofstream branchFile(refPath, std::ios::trunc | std::ios::binary);
        if (!branchFile) {
            std::cerr << "could not open branch file";
        }
        branchFile << hash;
    }
    else{
        std::ofstream headFile(headFilePath, std::ios::trunc | std::ios::binary);
        if(!headFile){
            std::cerr << "could not open HEAD file";
        }
        headFile << hash;
    }
}