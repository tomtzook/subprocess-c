cmake_minimum_required(VERSION 3.10)
project(subprocess C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

add_library(subprocess STATIC subprocess.c subprocess.h)
add_library(subprocess_cxx STATIC cxx/subprocess.cpp cxx/subprocess.h cxx/pipes.cpp cxx/sharedmem.cpp)

add_subdirectory(examples)