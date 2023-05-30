#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <cstring>
#include <mutex>
#include <pthread.h>
#include <thread>


#include "../src/flo-shani.h"

namespace fs = std::filesystem;

[[noreturn]] void printUsageAndExit(int status = EXIT_SUCCESS) {
    std::cout << "Usage: dedup [DIR]..\n";
    std::cout << "  Detects duplicated files in the given directory(s) recursively.\n";
    exit(status);
}

std::vector<fs::path> gatherFiles(int argc, char* argv[]) {
    std::unordered_set<std::string> seen;
    std::vector<fs::path> dirs; // We'll treat this as a stack
    for (int i = 1; i < argc; ++i) {
        auto p = fs::canonical(fs::path{argv[i]}); // This can throw exception, which is fine
        if (seen.count(p.c_str()) != 0) continue;
        seen.insert(p.c_str());
        dirs.push_back(p);
    }
    // Walk the directories
    std::vector<fs::path> files;
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

std::string digestToStr(unsigned char digest[]) {
    char buf[65];
    buf[64] = 0;
    for (unsigned i = 0; i < 32; ++i) {
        sprintf(buf + i * 2, "%02x", digest[i]);
    }
    return std::string(buf);
}

void workerThread(int i, std::mutex& mutex, std::vector<fs::path>& files, std::unordered_map<std::string, std::vector<fs::path>>& results) {
    while(true) {
        fs::path file;
        {
            std::lock_guard<std::mutex> lock(mutex);
            if(files.empty())
                return;
            file = files.back();
            files.pop_back();
        }
        uint8_t digest[32];
        memset(digest, 0, 32);
        uintmax_t size = fs::file_size(file);
        if (size == 0) {
            sha256_update_shani(nullptr, 0, digest);
            auto hash = digestToStr(digest);
            if (results.count(hash) == 0) {
                results[hash] = std::vector{file};
                continue;
            }
            results[hash].push_back(file);
            continue;
        }

        std::ifstream ifs(file, std::ios::binary);
        if (!ifs) {
            std::cerr << "Couldn't open file: " << file.c_str() << std::endl;
            continue;
        }
        char* bytes = (char* ) aligned_alloc(32, size);
        ifs.read(bytes, size);
        sha256_update_shani(reinterpret_cast<unsigned char*>(bytes), size, digest);
        free(bytes);
        auto hash = digestToStr(digest);
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (results.count(hash) == 0) {
                results[hash] = std::vector{file};
                continue;
            }
            results[hash].push_back(file);
        }
    }
}

int main(int argc, char* argv[]) {
    // Check arguments
    if (argc < 2) {
        printUsageAndExit();
    }
    for (int i = 1; i < argc; ++i) {
        fs::path p{argv[i]};
        if (!fs::is_directory(p)) {
            std::cerr << argv[i] << " is not a directory.\n";
            printUsageAndExit(EXIT_FAILURE);
        }
    }
    // Gather files
    std::cout << "Gathering files...\n\n";
    std::vector<fs::path> files = gatherFiles(argc, argv);
    std::cout << "Gathered " << files.size() << " files\n\n";
    // Calculate hashes
    std::cout << "Calculating SHA-256 hashes..\n\n";
    std::mutex m;
    std::unordered_map<std::string, std::vector<fs::path>> visitedFiles;
    std::vector<std::thread> threads;
    for(int i=0; i<10; ++i) {
        threads.push_back(std::thread(workerThread, i, std::ref(m), std::ref(files), std::ref(visitedFiles)));
    }
    for(auto& t : threads) {
        t.join();
    }


//
    // Print results                                    
    std::cout << "Duplicates:\n";
    for (auto& f : visitedFiles) {
        if (f.second.size() == 1) continue;
        std::cout << "Hash: " << f.first << "\n";
        for (size_t i = 0; i < f.second.size(); ++i) {
            std::cout << f.second[i] << "\n";
        }
        std::cout << "---\n";
    }
    return 0;
}
