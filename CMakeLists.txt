cmake_minimum_required(VERSION 3.14)
project(subprocess C)

set(CMAKE_C_STANDARD 11)

add_library(subprocess STATIC subprocess.c subprocess.h test.c)


add_executable(test test.c)
target_link_libraries(test subprocess)