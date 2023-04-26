#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;


[[noreturn]] void printUsageAndExit(int status= EXIT_SUCCESS) {
    std::cout << "Usage: twin-seeker [DIR]..\n";
    std::cout << " Detects duplicated files in the given directory(s) recursively.\n";
    exit(status); //exit with a non-zero status code to signal that an error occurred.
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
}