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

std::vector<fs::path> gatherFiles(const std::vector<std::string>& directories) {
    std::unordered_set<std::string> seen;
    std::vector<fs::path> dirs; // We'll treat this as a stack
    for (const auto& directory : directories) {
        auto p = fs::canonical(fs::path{directory}); // This can throw an exception, which is fine
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

void processFile(const fs::path& file, std::unordered_map<std::string, std::vector<fs::path>>& results) {
    uint8_t digest[32];
    memset(digest, 0, 32);
    uintmax_t size = fs::file_size(file);
    if (size == 0) {
        sha256_update_shani(nullptr, 0, digest);
        auto hash = digestToStr(digest);
        if (results.count(hash) == 0) {
            results[hash] = std::vector{file};
            return;
        }
        results[hash].push_back(file);
        return;
    }

    std::ifstream ifs(file, std::ios::binary);
    if (!ifs) {
        std::cerr << "Couldn't open file: " << file.c_str() << std::endl;
        return;
    }
    char* bytes = (char*)aligned_alloc(32, size);
    ifs.read(bytes, size);
    sha256_update_shani(reinterpret_cast<unsigned char*>(bytes), size, digest);
    free(bytes);
    auto hash = digestToStr(digest);

    if (results.count(hash) == 0) {
        results[hash] = std::vector{file};
    } else {
        results[hash].push_back(file);
    }
}

void workerThread(int id, std::mutex& mutex, std::vector<fs::path>& files, std::unordered_map<std::string, std::vector<fs::path>>& results) {
    while (true) {
        fs::path file;
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (files.empty())
                return;
            file = files.back();
            files.pop_back();
        }
        processFile(file, results);
    }
}

void calculateHashes(std::vector<fs::path>& files, std::unordered_map<std::string, std::vector<fs::path>>& results) {
    std::mutex mutex;
    std::vector<std::thread> threads;
    const int numThreads = std::thread::hardware_concurrency();
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(workerThread, i, std::ref(mutex), std::ref(files), std::ref(results));
    }
    for (auto& t : threads) {
        t.join();
    }
}

void printDuplicates(const std::unordered_map<std::string, std::vector<fs::path>>& visitedFiles) {
    std::cout << "Duplicates:\n";
    for (const auto& f : visitedFiles) {
        if (f.second.size() == 1) continue;
        std::cout << "Hash: " << f.first << "\n";
        for (const auto& file : f.second) {
            std::cout << file << "\n";
        }
        std::cout << "---\n";
    }
}

int main(int argc, char* argv[]) {
    // Check arguments
    if (argc < 2) {
        printUsageAndExit();
    }
    std::vector<std::string> directories(argv + 1, argv + argc);
    for (const auto& directory : directories) {
        fs::path p{directory};
        if (!fs::is_directory(p)) {
            std::cerr << directory << " is not a directory.\n";
            printUsageAndExit(EXIT_FAILURE);
        }
    }
    // Gather files
    std::cout << "Gathering files...\n\n";
    std::vector<fs::path> files = gatherFiles(directories);
    std::cout << "Gathered " << files.size() << " files\n\n";
    // Calculate hashes
    std::cout << "Calculating SHA-256 hashes..\n\n";
    std::unordered_map<std::string, std::vector<fs::path>> visitedFiles;
    calculateHashes(files, visitedFiles);
    // Print results
    printDuplicates(visitedFiles);

    return 0;
}
