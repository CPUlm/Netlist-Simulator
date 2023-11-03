function(add_negative_test INPUT)
  set(options FAIL)
  set(oneValueArgs)
  set(multiValueArgs)
  cmake_parse_arguments(ADD_NEGATIVE_TEST "${options}"
          "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  add_test(NAME negative.${INPUT}
          COMMAND ${CMAKE_COMMAND}
          -DNETLIST_EXE=$<TARGET_FILE:netlist>
          -DINPUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/${INPUT}
          -P ${CMAKE_CURRENT_SOURCE_DIR}/RunTest.cmake
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )
endfunction()

add_negative_test(constant_in_reg.net)
add_negative_test(cycle.net)
add_negative_test(extra_comma_input_list.net)
add_negative_test(extra_comma_output_list.net)
add_negative_test(extra_comma_var_list.net)
add_negative_test(indirect_cycle.net)
add_negative_test(missing_in.net)
add_negative_test(missing_input.net)
add_negative_test(missing_input_in_var_list.net)
add_negative_test(missing_output.net)
add_negative_test(missing_output_in_var_list.net)
add_negative_test(missing_var.net)
add_negative_test(multiple_same_inputs.net)
add_negative_test(multiple_same_outputs.net)
add_negative_test(multiple_same_variables.net)
add_negative_test(nul_bus_size.net)
add_negative_test(too_big_bus_size.net)
