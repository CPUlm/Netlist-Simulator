cmake_minimum_required(VERSION 3.16)

project(netlist)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(fmt)

add_library(netlistlib
        src/lexer.hpp
        src/token.hpp
        src/lexer.cpp
        src/keywords.hpp
        src/graph.hpp
        src/parser.hpp
        src/parser.cpp
        src/program.hpp
        src/program_printer.hpp
        src/program_printer.cpp
        src/report.hpp
        src/report.cpp
)

target_link_libraries(netlistlib PUBLIC fmt::fmt)

add_executable(netlist
        src/main.cpp
)

target_link_libraries(netlist PUBLIC netlistlib)
target_include_directories(netlistlib PUBLIC src/)

# gperf (GNU perfect hash function generator)
# It is only required if the keywords.def file is modified.
find_program(GPERF_PROGRAM gperf)
if (GPERF_PROGRAM)
  add_custom_command(
          OUTPUT src/keywords.hpp
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ${GPERF_PROGRAM} -t --output-file=src/keywords.hpp src/keywords.def
          COMMENT "Generating the keywords perfect hash function"
          DEPENDS src/keywords.def
  )
endif ()

enable_testing()
add_subdirectory(unittest)
