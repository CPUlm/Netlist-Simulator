cmake_minimum_required(VERSION 3.16)

project(netlist)

add_executable(netlist src/main.cpp)

enable_testing()
add_subdirectory(unittest)
