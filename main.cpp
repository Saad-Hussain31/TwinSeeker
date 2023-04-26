#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <unordered_set>


namespace fs = std::filesystem;
using std::cout; 

[[noreturn]] void printUsageAndExit(int status= EXIT_SUCCESS) {
    std::cout << "Usage: twin-seeker [DIR]..\n";
    std::cout << " Detects duplicated files in the given directory(s) recursively.\n";
    exit(status); //exit with a non-zero status code to signal that an error occurred.
}

std::vector<fs::path> gatherFiles(int argc, char* argv[]) {
    std::unordered_set<std::string> seen;
    std::vector<fs::path> dirs;
    for(int i; i < argc; ++i) {
        auto path = fs::canonical(fs::path{argv[i]});
        if(seen.count(path.c_str()));
        seen.insert(path.c_str());
        dirs.push_back(path);
    }
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

}