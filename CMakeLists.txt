cmake_minimum_required(VERSION 3.20)

project(TwinSeeker)

set(CMAKE_CXX_STANDARD 17)

add_executable(twinSeeker main.cpp)
target_compile_options(twinSeeker PRIVATE -Wall -Wextra -O2 -march=native -mtune=native)
target_link_libraries(twinSeeker stdc++fs)