cmake_minimum_required(VERSION 3.20)

project(TwinSeeker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)


add_executable(twinSeeker src/main.cpp)
target_compile_options(twinSeeker PRIVATE -Wall -Wextra -O2 -march=native -mtune=native)
target_link_libraries(twinSeeker stdc++fs)