cmake_minimum_required(VERSION 3.10)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include)

add_executable(subprocess_example_echo_cxx example_echo.cpp)
target_link_libraries(subprocess_example_echo_cxx subprocess_cxx)

add_executable(subprocess_example_func_sharedmem_cxx example_func_sharedmem.cpp)
target_link_libraries(subprocess_example_func_sharedmem_cxx subprocess_cxx)
