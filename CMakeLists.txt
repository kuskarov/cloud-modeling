cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

project(simulator)

# include external dependencies
add_subdirectory(external)

# after that, build the project
add_subdirectory(src)
