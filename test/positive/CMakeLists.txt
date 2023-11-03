function(add_positive_test INPUT)
    set(options FAIL)
    set(oneValueArgs)
    set(multiValueArgs)
    cmake_parse_arguments(ADD_POSITIVE_TEST "${options}"
            "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_test(NAME positive.${INPUT}
            COMMAND ${CMAKE_COMMAND}
            -DNETLIST_EXE=$<TARGET_FILE:netlist>
            -DINPUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/${INPUT}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/RunTest.cmake
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endfunction()

add_positive_test(and.net)
add_positive_test(binary_constants.net)
add_positive_test(clock_div.net)
add_positive_test(cm2.net)
add_positive_test(comments.net)
add_positive_test(decimal_constants.net)
add_positive_test(empty_equation_list.net)
add_positive_test(empty_input_list.net)
add_positive_test(fulladder.net)
add_positive_test(hexadecimal_constants.net)
add_positive_test(identifiers.net)
add_positive_test(mux.net)
add_positive_test(nadder.net)
add_positive_test(nand.net)
add_positive_test(nor.net)
add_positive_test(not.net)
add_positive_test(or.net)
add_positive_test(ram.net)
add_positive_test(reg.net)
add_positive_test(select.net)
add_positive_test(slice.net)
add_positive_test(xnor.net)
add_positive_test(xor.net)
