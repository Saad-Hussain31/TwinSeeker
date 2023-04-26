#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <unordered_set>
#include <unordered_map>


namespace fs = std::filesystem;
using std::cout; 

std::string sha256(const std::vector<char>& input);

[[noreturn]] void printUsageAndExit(int status= EXIT_SUCCESS) {
    std::cout << "Usage: twin-seeker [DIR]..\n";
    std::cout << " Detects duplicated files in the given directory(s) recursively.\n";
    exit(status); //exit with a non-zero status code to signal that an error occurred.
}

std::vector<fs::path> gatherFiles(int argc, char* argv[]) {
    std::unordered_set<std::string> seen; //to keep track of the directories already processed 
    std::vector<fs::path> dirs; //stack for processing directories
    for(int i = 1; i < argc; ++i) {
        auto path = fs::canonical(fs::path{argv[i]}); //removes symlinks, emojis etc
        if(seen.count(path.c_str()) >= 0) continue;
        seen.insert(path.c_str());
        dirs.push_back(path);
    }

    //walking the directories
    std::vector<fs::path> files; //to store the file paths found during the traversal
    while(!dirs.empty()) {
        fs::path dir = dirs.back();
        dirs.pop_back();
        for (auto it = fs::directory_iterator(dir); it != fs::directory_iterator(); ++it) {

            if(fs::is_directory(*it)) {
                fs::path newDir = fs::canonical(*it);
                if (seen.count(newDir.c_str()) != 0) continue;
                dirs.push_back(newDir);
                seen.insert(newDir.c_str());
                continue;
            }
            if(fs::is_regular_file(*it)) {
                files.push_back(fs::canonical(*it));
            }
        }
    }
    return files; //canonical paths of all files in the input directories and their subdirectories
}

int main(int argc, char* argv[]) {
    if(argc < 2) {
        printUsageAndExit();
    }
    
    for(int i = 1; i < argc; ++i) {
        fs::path path{argv[i]};
        if(!fs::is_directory(path)) { //checks if 'path' represents a valid directory.
            std::cerr << argv[i] << " is not a directory.\n";
            printUsageAndExit(EXIT_FAILURE);
        }
    }

    //gather files
    cout << "Gathering files...\n\n";
    std::vector<fs::path> files = gatherFiles(argc, argv);
    cout << "Gathered " << files.size() << " files\n\n";

    //calculate hashes
    std::unordered_map<std::string, std::vector<fs::path>> visitedFiles; //key is hash and val is file vector. single vec if no dups

    for(auto& file : files) {
        uintmax_t size = fs::file_size(file);
        if(size == 0) {
            std::vector<char> empty;
            auto hash = sha256(empty);
            if(visitedFiles.count(hash) == 0) {
                visitedFiles[hash] = std::vector{file};
                continue;
            }
            visitedFiles[hash].push_back(file); //adding for duplicate
            continue;
        }
    }


}