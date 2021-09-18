cmake_minimum_required(VERSION 3.10)
project(subprocess C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

include_directories(include)

add_library(subprocess STATIC
        src/subprocess.c)

add_library(subprocess_cxx STATIC
        src/cxx/subprocess.cpp
        src/cxx/pipes.cpp
        src/cxx/sharedmem.cpp)

add_subdirectory(examples)