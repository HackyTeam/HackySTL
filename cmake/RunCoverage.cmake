# Runs the test suite, then writes a gcov HTML report with gcovr.
# The `coverage` target runs this script with these variables:
#   CTEST_COMMAND  the ctest executable
#   GCOVR          the gcovr executable
#   GCOV           the gcov executable that matches the compiler
#   SOURCE_DIR     the project source directory
#   BINARY_DIR     the build directory
#
# Counters from earlier runs are removed first, so the report covers only this run.
# The coverage target builds the project first, so the counters match the current code.

file(GLOB_RECURSE _counter_files "${BINARY_DIR}/*.gcda")
if(_counter_files)
    file(REMOVE ${_counter_files})
endif()

execute_process(
    COMMAND "${CTEST_COMMAND}" --test-dir "${BINARY_DIR}" --output-on-failure
    WORKING_DIRECTORY "${BINARY_DIR}"
    RESULT_VARIABLE _tests_result
)
if(NOT _tests_result EQUAL 0)
    message(FATAL_ERROR "The tests failed, so no coverage report was written")
endif()

# gcovr matches --filter against absolute paths, so the library is selected by its directory.
file(MAKE_DIRECTORY "${BINARY_DIR}/coverage")
execute_process(
    COMMAND
        "${GCOVR}"
        --root "${SOURCE_DIR}"
        --object-directory "${BINARY_DIR}"
        --gcov-executable "${GCOV}"
        --filter ".*/lib/.*"
        --html-details "${BINARY_DIR}/coverage/index.html"
        --print-summary
    WORKING_DIRECTORY "${BINARY_DIR}"
    RESULT_VARIABLE _report_result
)
if(NOT _report_result EQUAL 0)
    message(FATAL_ERROR "gcovr failed to write the coverage report")
endif()

message(STATUS "Coverage report: ${BINARY_DIR}/coverage/index.html")
