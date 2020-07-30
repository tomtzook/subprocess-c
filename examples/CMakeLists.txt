cmake_minimum_required(VERSION 3.10)

add_executable(example_func example_func.c)
target_link_libraries(example_func subprocess)

add_executable(example_shell example_shell.c)
target_link_libraries(example_shell subprocess)
