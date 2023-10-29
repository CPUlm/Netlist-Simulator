include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/e47544ad31cb3ceecd04cc13e8fe556f8df9fe0b.zip
        DOWNLOAD_EXTRACT_TIMESTAMP false
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(
        netlist_test
        lexer_test.cpp
        graph_test.cpp
        parser_test.cpp
        line_map_test.cpp
        report_test.cpp
        simulator_test.cpp
        bytecode_writer_test.cpp
        dependency_graph_test.cpp
)

target_link_libraries(
        netlist_test
        netlistlib
        GTest::gtest_main
        GTest::gmock_main
)

include(GoogleTest)
gtest_discover_tests(netlist_test)
