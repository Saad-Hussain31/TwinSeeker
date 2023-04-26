cmake_minimum_required(VERSION 3.20)

project(TwinSeeker)

set(CMAKE_CXX_STANDARD 17)

add_executable(twinSeeker main.cpp)
target_link_libraries(twinSeeker stdc++fs)