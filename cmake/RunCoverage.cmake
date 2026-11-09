# Runs the test suite, then writes a gcov HTML report with gcovr.
# The `coverage` target runs this script with these variables:
#   CTEST_COMMAND  the ctest executable
#   GCOVR          the gcovr executable
#   GCOV           the gcov executable that matches the compiler
#   SOURCE_DIR     the project source directory
#   BINARY_DIR     the build directory
#   PYTHON         optional; the Python interpreter for the per-site summary
#
# Counters from earlier runs are removed first, so the report covers only this run.
# The coverage target builds the project first, so the counters match the current code.

# Build first, so that the test programs match the sources even when the build was not run.
execute_process(
    COMMAND "${CMAKE_COMMAND}" --build "${BINARY_DIR}" --parallel
    RESULT_VARIABLE _build_result
)
if(NOT _build_result EQUAL 0)
    message(FATAL_ERROR "The build failed, so no coverage report was written")
endif()

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

# The per-site summary counts each line, branch and function once, however many template
# instances it has. It reads the same data as JSON, which is kept beside the report folder
# so that the uploaded report does not include it. It runs only when Python is available.
if(PYTHON)
    execute_process(
        COMMAND
            "${GCOVR}"
            --root "${SOURCE_DIR}"
            --object-directory "${BINARY_DIR}"
            --gcov-executable "${GCOV}"
            --filter ".*/lib/.*"
            --json "${BINARY_DIR}/coverage.json"
        WORKING_DIRECTORY "${BINARY_DIR}"
        RESULT_VARIABLE _json_result
    )
    if(NOT _json_result EQUAL 0)
        message(FATAL_ERROR "gcovr failed to write the JSON coverage data")
    endif()

    execute_process(
        COMMAND "${PYTHON}" "${SOURCE_DIR}/cmake/CoverageSites.py" "${BINARY_DIR}/coverage.json"
        RESULT_VARIABLE _sites_result
    )
    if(NOT _sites_result EQUAL 0)
        message(FATAL_ERROR "The per-site coverage summary failed")
    endif()
endif()
