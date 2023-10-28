include(FetchContent)
FetchContent_Declare(
        googletest
        DOWNLOAD_EXTRACT_TIMESTAMP false
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(
        netlist_test
        lexer_test.cpp
        report_test.cpp
        parser_test.cpp
        graph_test.cpp
)

target_link_libraries(
        netlist_test
        netlistlib
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(netlist_test)
