cmake_minimum_required(VERSION 3.16)

project(netlist)

# C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(netlist_lib
        src/lexer.hpp
        src/token.hpp
        src/lexer.cpp
        src/keywords.hpp
        src/parser.hpp
        src/parser.cpp
        src/line_map.hpp
        src/line_map.cpp
        src/report.hpp
        src/report.cpp
        src/program.cpp
        src/simulator/simulator.hpp
        src/simulator/simulator.cpp
        src/simulator/interpreter_backend.hpp
        src/simulator/interpreter_backend.cpp
        src/dependency_graph.hpp
        src/dependency_graph.cpp
)

add_subdirectory(fmt)
target_link_libraries(netlist_lib PUBLIC fmt::fmt)

add_executable(netlist
        src/main.cpp
)

target_link_libraries(netlist PUBLIC netlist_lib)
target_include_directories(netlist_lib PUBLIC src/)

# Automatic Doxygen documentation generation.
find_package(Doxygen COMPONENTS dot)
if (DOXYGEN_FOUND)
  add_custom_target(netlist_doc DEPENDS Doxygen::doxygen COMMAND Doxygen::doxygen doc/Doxyfile WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif ()

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
add_subdirectory(test)
