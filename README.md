# TwinSeeker

TwinSeeker is a C++ based command-line utility that recursively scans given directories and identifies duplicate files. By utilizing SHA-256 hashing, this tool efficiently compares files to help free up storage space by identifying redundant files, making it easier to manage your data.

## Features

- Recursively scans directories for duplicate files.
- Efficient comparison of files using SHA-256 hashing.
- Handles symbolic links and ignores identical directory paths.
- Handles file read errors gracefully.
- Reports zero byte files (empty files) as duplicates.

## Prerequisites

- A C++17 compliant compiler
- [CMake](https://cmake.org/) version 3.10 or above

## Installation

1. Clone this repository to your local machine.

    ```
    git clone https://github.com/Saad-Hussain31/TwinSeeker.git
    ```

2. Navigate into the TwinSeeker directory and create a build directory.

    ```
    cd TwinSeeker
    mkdir build
    ```

3. Change to the build directory, run CMake to configure the build, and then compile the code.

    ```
    cd build
    cmake ..
    make
    ```

4. The `twin-seeker` executable will be created in the `build` directory.

## Usage

Run `twin-seeker` followed by the directories you want to scan. The tool will recursively scan the directories and output any duplicate files it finds. Here is a basic example:

./twin-seeker directory1 directory2


In case you provide an argument that is not a directory, TwinSeeker will print an error message and exit. In case a file cannot be read, TwinSeeker will print an error message but continue scanning other files.

_Note: Replace `directory1` and `directory2` with the actual paths to directories you want to scan._


