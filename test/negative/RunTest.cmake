cmake_policy(SET CMP0012 NEW)

execute_process(COMMAND ${NETLIST_EXE} --schedule ${INPUT_FILE} RESULT_VARIABLE CMD_RESULT)
message(STATUS "netlist exited with code ${CMD_RESULT}")
if (NOT CMD_RESULT)
  message(FATAL_ERROR "Succeeded to parse ${INPUT_FILE}, we expected a fail.")
endif ()
