#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <unordered_set>
#include <unordered_map>
#include "sha256.hpp"

namespace fs = std::filesystem;
using std::cout;

[[noreturn]] void printUsageAndExit(int status = EXIT_SUCCESS) {
    std::cout << "Usage: twin-seeker [DIR]..\n";
    std::cout << " Detects duplicated files in the given directory(s) recursively.\n";
    exit(status);
}

std::vector<fs::path> gatherFilesRecursive(const fs::path& directory, std::unordered_set<std::string>& seen) {
    std::vector<fs::path> files;

    for (const auto& entry : fs::recursive_directory_iterator(directory)) {
        const fs::path& path = entry.path();
        if (fs::is_directory(path)) {
            fs::path canonicalPath = fs::canonical(path);
            if (seen.count(canonicalPath.c_str()) != 0) {
                continue;
            }
            seen.insert(canonicalPath.c_str());
            auto subFiles = gatherFilesRecursive(canonicalPath, seen);
            files.insert(files.end(), subFiles.begin(), subFiles.end());
        }
        else if (fs::is_regular_file(path)) {
            files.push_back(path);
        }
    }

    return files;
}

std::vector<fs::path> gatherFiles(int argc, char* argv[]) {
    std::unordered_set<std::string> seen;
    std::vector<fs::path> files;

    for (int i = 1; i < argc; ++i) {
        fs::path directory{ argv[i] };
        if (!fs::is_directory(directory)) {
            std::cerr << argv[i] << " is not a directory.\n";
            printUsageAndExit(EXIT_FAILURE);
        }

        fs::path canonicalPath = fs::canonical(directory);
        if (seen.count(canonicalPath.c_str()) != 0) {
            continue;
        }
        seen.insert(canonicalPath.c_str());
        auto subFiles = gatherFilesRecursive(canonicalPath, seen);
        files.insert(files.end(), subFiles.begin(), subFiles.end());
    }

    return files;
}

std::string calculateFileHash(const fs::path& file) {
    uintmax_t size = fs::file_size(file);
    if (size == 0) {
        return "";
    }

    std::ifstream ifs(file, std::ios::binary);
    if (!ifs) {
        std::cerr << "Could not open file: " << file.c_str() << std::endl;
        return "";
    }

    std::vector<char> bytes(size);
    ifs.read(bytes.data(), size);
    return sha256(bytes);
}

void findDuplicateFiles(const std::vector<fs::path>& files, std::unordered_map<std::string, std::vector<fs::path>>& visitedFiles) {
    for (const auto& file : files) {
        std::string hash = calculateFileHash(file);
        if (hash.empty()) {
            visitedFiles[hash].push_back(file);
            continue;
        }

        if (visitedFiles.count(hash) == 0) {
            visitedFiles[hash] = std::vector<fs::path>{file};
        }
        else {
            visitedFiles[hash].push_back(file);
        }
    }
}

void printDuplicates(const std::unordered_map<std::string, std::vector<fs::path>>& visitedFiles) {
    cout << "Duplicates:\n";
    for (const auto& f : visitedFiles) {
        if (f.second.size() == 1) {
            continue;
        }
        cout << "Hash: " << f.first << "\n";
        for (const auto& file : f.second) {
            cout << file << "\n";
        }
        cout << "---\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsageAndExit();
    }

    std::vector<fs::path> files = gatherFiles(argc, argv);
    cout << "Gathering files...\n\n";
    cout << "Gathered " << files.size() << " files\n\n";

    std::unordered_map<std::string, std::vector<fs::path>> visitedFiles;
    findDuplicateFiles(files, visitedFiles);

    printDuplicates(visitedFiles);

    return 0;
}
