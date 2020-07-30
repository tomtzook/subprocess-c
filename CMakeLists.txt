cmake_minimum_required(VERSION 3.10)
project(subprocess C)

set(CMAKE_C_STANDARD 11)

add_library(subprocess STATIC subprocess.c subprocess.h)

add_subdirectory(examples)