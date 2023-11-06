cmake_minimum_required(VERSION 3.16)

project(netlist)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(fmt)

add_library(netlistlib
        src/utilities.cpp
        src/utilities.hpp

        src/program.hpp
        src/program.cpp

        src/token.hpp

        src/lexer.hpp
        src/lexer.cpp

        src/parser.hpp
        src/parser.cpp

        src/report.hpp
        src/report.cpp

        src/graph.hpp
        src/scheduler.hpp

        src/dot_printer.cpp
        src/dot_printer.hpp

        src/program_printer.hpp
        src/program_printer.cpp

        src/simulator.cpp
        src/simulator.hpp

        src/input_manager.cpp
        src/input_manager.hpp

        src/command_line_parser.cpp
        src/command_line_parser.hpp)

target_link_libraries(netlistlib PUBLIC fmt::fmt)

add_executable(netlist src/main.cpp)

target_link_libraries(netlist PUBLIC netlistlib)
target_include_directories(netlistlib PUBLIC src/)

enable_testing()
add_subdirectory(unittest)
