cmake_minimum_required(VERSION 3.1)
project(TinyShell C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)
add_compile_options(-Wno-unused)

add_executable(TinyShell TinyShell.c)
target_link_libraries(TinyShell)
