cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

project(simulator CXX)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(external)

add_subdirectory(src)
