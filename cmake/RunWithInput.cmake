if(NOT DEFINED PROGRAM)
    message(FATAL_ERROR "PROGRAM must name the executable to run")
endif()

if(NOT DEFINED TIMEOUT)
    set(TIMEOUT 15)
endif()

set(execute_options
    COMMAND "${PROGRAM}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE errors
    TIMEOUT "${TIMEOUT}"
)

if(DEFINED WORKING_DIRECTORY)
    list(APPEND execute_options WORKING_DIRECTORY "${WORKING_DIRECTORY}")
endif()

if(DEFINED INPUT_FILE)
    list(APPEND execute_options INPUT_FILE "${INPUT_FILE}")
endif()

if(DEFINED FIXTURE_FILE AND DEFINED WORKING_DIRECTORY)
    file(COPY "${FIXTURE_FILE}" DESTINATION "${WORKING_DIRECTORY}")
endif()

execute_process(${execute_options})

if(NOT result STREQUAL "0")
    message("${output}${errors}")
    message(FATAL_ERROR "Interactive test failed with result: ${result}")
endif()