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

    ```shell
    git clone https://github.com/Saad-Hussain31/TwinSeeker.git
    ```

2. Navigate into the TwinSeeker directory and execute the `generate_samples.sh` script.

    ```shell
    cd TwinSeeker
    ./generate_samples.sh
    ```

    This script generates sample files in the `samples` directory for testing TwinSeeker.

3. Create a build directory.

    ```shell
    mkdir build
    ```

4. Change to the build directory, run CMake to configure the build, and then compile the code.

    ```shell
    cd build
    cmake ..
    make
    ```

5. The `twin-seeker` executable will be created in the `build` directory.

## Usage

Run `twin-seeker` followed by the directories you want to scan. The tool will recursively scan the directories and output any duplicate files it finds. Here is a basic example:

```shell
./twin-seeker directory1 directory2
