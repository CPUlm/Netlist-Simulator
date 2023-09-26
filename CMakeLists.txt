cmake_minimum_required(VERSION 3.16)

project(netlist)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(netlistlib
        src/lexer.hpp
        src/token.hpp
        src/lexer.cpp
        src/identifier_table.hpp
        src/bump_allocator.hpp
        src/identifier_table.cpp
)

add_executable(netlist
        src/main.cpp
)

target_link_libraries(netlist PUBLIC netlistlib)
target_include_directories(netlistlib PUBLIC src/)

enable_testing()
add_subdirectory(unittest)
