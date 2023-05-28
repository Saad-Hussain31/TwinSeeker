#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <cstring>

// #include <sha256.h> // Let's treat this as a black box
#include "../src/flo-shani.h"

using namespace std;
namespace fs = std::filesystem;

[[noreturn]] void printUsageAndExit(int status = EXIT_SUCCESS) {
    cout << "Usage: dedup [DIR]..\n";
    cout << "  Detects duplicated files in the given directory(s) recursively.\n";
    exit(status);
}

vector<fs::path> gatherFiles(int argc, char* argv[]) {
    unordered_set<string> seen;
    vector<fs::path> dirs; // We'll treat this as a stack
    for (int i = 1; i < argc; ++i) {
        auto p = fs::canonical(fs::path{argv[i]}); // This can throw exception, which is fine
        if (seen.count(p.c_str()) != 0) continue;
        seen.insert(p.c_str());
        dirs.push_back(p);
    }
    // Walk the directories
    vector<fs::path> files;
    while (!dirs.empty()) {
        fs::path d = dirs.back();
        dirs.pop_back();
        for (auto it = fs::directory_iterator(d); it != fs::directory_iterator(); ++it) {
            // Two states we care about: 1, directory 2, regular file
            if (fs::is_directory(*it)) {
                fs::path newDir = fs::canonical(*it);
                if (seen.count(newDir.c_str()) != 0) continue;
                dirs.push_back(newDir);
                seen.insert(newDir.c_str());
                continue;
            }
            if (fs::is_regular_file(*it)) {
                files.push_back(fs::canonical(*it));
            }
        }
    }
    return files;
}

string digestToStr(unsigned char digest[]) {
    char buf[65];
    buf[64] = 0;
    for (unsigned i = 0; i < 32; ++i) {
        sprintf(buf + i * 2, "%02x", digest[i]);
    }
    return string(buf);
}

int main(int argc, char* argv[]) {
    // Check arguments
    if (argc < 2) {
        printUsageAndExit();
    }
    for (int i = 1; i < argc; ++i) {
        fs::path p{argv[i]};
        if (!fs::is_directory(p)) {
            cerr << argv[i] << " is not a directory.\n";
            printUsageAndExit(EXIT_FAILURE);
        }
    }
    // Gather files
    cout << "Gathering files...\n\n";
    vector<fs::path> files = gatherFiles(argc, argv);
    cout << "Gathered " << files.size() << " files\n\n";
    // Calculate hashes
    cout << "Calculating SHA-256 hashes..\n\n";
    unordered_map<string, vector<fs::path>> visitedFiles; // Key: SHA-256 hash Value: list of files
    for (auto& file : files) {
        uint8_t digest[32];
        memset(digest, 0, 32);
        uintmax_t size = fs::file_size(file);
        if (size == 0) {
            sha256_update_shani(nullptr, 0, digest);
            auto hash = digestToStr(digest);
            if (visitedFiles.count(hash) == 0) {
                visitedFiles[hash] = vector{file};
                continue;
            }
            visitedFiles[hash].push_back(file);
            continue;
        }
        ifstream ifs(file, ios::binary);
        if (!ifs) {
            cerr << "Couldn't open file: " << file.c_str() << std::endl;
            continue; // Move on
        }
        char* bytes = (char *) aligned_alloc(32, size); // 128, 256 bits long registers
        ifs.read(bytes, size);
        sha256_update_shani(reinterpret_cast<unsigned char*>(bytes), size, digest);
        free(bytes);
        auto hash = digestToStr(digest);
        if (visitedFiles.count(hash) == 0) {
            visitedFiles[hash] = vector{file};
            continue;
        }
        visitedFiles[hash].push_back(file);
    }
    // Print results
    cout << "Duplicates:\n";
    for (auto& f : visitedFiles) {
        if (f.second.size() == 1) continue;
        cout << "Hash: " << f.first << "\n";
        for (size_t i = 0; i < f.second.size(); ++i) {
            cout << f.second[i] << "\n";
        }
        cout << "---\n";
    }
    return 0;
}
