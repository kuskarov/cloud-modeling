cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

project(simulator CXX)

set(CMAKE_CXX_STANDARD 20)

# include external dependencies
add_subdirectory(external)

# after that, build the project
add_subdirectory(src)
