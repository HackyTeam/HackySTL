cmake_minimum_required(VERSION 3.23)

# CMake 4.0 moves the MSVC runtime checks into the MSVC_RUNTIME_CHECKS property
# (CMP0184). The profiling targets clear that property, see below.
if(POLICY CMP0184)
    cmake_policy(SET CMP0184 NEW)
endif()

project(HackySTL LANGUAGES CXX)

if(MSVC)
    # CMake's default flags turn exceptions on with /EHsc. The library is built
    # without them, so the default is replaced here. Overriding it per target
    # instead makes MSVC report warning D9025 for every file.
    string(REPLACE "/EHsc" "/EHs-c-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

include(CTest)
find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(
    HACKYSTL_ENABLE_SANITIZERS
    "Build the test executables with AddressSanitizer and UndefinedBehaviorSanitizer"
    ON
)

set(
    HACKYSTL_INTERACTIVE_TIMEOUT 15 CACHE STRING
    "Timeout for interactive CTest wrappers"
)

# Keep compiler and linker policy in one target so every test uses the same
# configuration without duplicating flags in each executable definition.
add_library(hackystl_test_config INTERFACE)
target_include_directories(
    hackystl_test_config
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/lib"
        "${CMAKE_CURRENT_SOURCE_DIR}/Tests"
)
target_link_libraries(hackystl_test_config INTERFACE Threads::Threads)

if(WIN32)
    # Winsock for the network code, the address-wait primitives behind futex
    # locks, and the shell path helpers. MinGW ignores the `#pragma comment`
    # directives that MSVC uses for the same libraries, so link them here.
    target_link_libraries(
        hackystl_test_config
        INTERFACE ws2_32 synchronization shlwapi
    )
endif()

if(MSVC)
    # The tests check their results with assert, so it stays active in every build type.
    # /utf-8 reads sources as UTF-8, so literals such as u8"€" mean the same as on GCC and Clang.
    # C4530 flags the try/catch blocks in standard headers, which this build does not use.
    target_compile_options(hackystl_test_config INTERFACE /utf-8 /W4 /wd4530 /UNDEBUG)
    # The tests build fixtures with the C stdio functions, which MSVC reports as unsafe.
    target_compile_definitions(hackystl_test_config INTERFACE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(
        hackystl_test_config
        INTERFACE
            -fno-exceptions
            -Wall
            -Wextra
            -Wempty-body
            -Wignored-qualifiers
            -Wmissing-field-initializers
            -Wstring-compare
            -Wuninitialized
            -Wunused-parameter
            # The tests check their results with assert, so it stays active in every build type.
            -UNDEBUG
    )
endif()

if(HACKYSTL_ENABLE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        target_compile_options(
            hackystl_test_config
            INTERFACE -fsanitize=address,undefined
        )
        target_link_options(
            hackystl_test_config
            INTERFACE -fsanitize=address,undefined
        )
    elseif(NOT MSVC)
        message(
            WARNING
            "HACKYSTL_ENABLE_SANITIZERS is not supported by this compiler"
        )
    endif()
endif()

option(
    HACKYSTL_ENABLE_COVERAGE
    "Instrument the tests and examples with gcov and add a coverage target that writes an HTML report"
    OFF
)

if(HACKYSTL_ENABLE_COVERAGE)
    # gcov is GCC's coverage tool. Clang's coverage output needs llvm-cov, which this build does not use.
    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(FATAL_ERROR "HACKYSTL_ENABLE_COVERAGE needs GCC, because it uses gcov")
    endif()

    target_compile_options(
        hackystl_test_config
        INTERFACE --coverage -fprofile-update=atomic
    )
    target_link_options(hackystl_test_config INTERFACE --coverage)
    # Tests that check abort() paths dump the counters in the child process, see Tests/Support/AbortSupport.hpp.
    target_compile_definitions(hackystl_test_config INTERFACE HACKYSTL_COVERAGE=1)

    # gcov must be the same major version as the compiler, because gcov rejects data
    # written by another version. CMake sets the full compiler version, not a major one.
    string(REGEX MATCH "^[0-9]+" _compiler_major "${CMAKE_CXX_COMPILER_VERSION}")

    # Debian and Ubuntu install gcov-<major> beside each GCC version, so try that name first.
    # Otherwise use the gcov that the compiler names for itself.
    execute_process(
        COMMAND "${CMAKE_CXX_COMPILER}" -print-prog-name=gcov
        OUTPUT_VARIABLE _gcov_from_compiler
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    unset(GCOV_EXECUTABLE CACHE)
    find_program(
        GCOV_EXECUTABLE
        NAMES "gcov-${_compiler_major}" "${_gcov_from_compiler}"
        REQUIRED
    )

    execute_process(
        COMMAND "${GCOV_EXECUTABLE}" --version
        OUTPUT_VARIABLE _gcov_version_text
    )
    string(REGEX MATCH "\\) ([0-9]+)\\." _gcov_version_match "${_gcov_version_text}")
    if(NOT CMAKE_MATCH_1 STREQUAL _compiler_major)
        message(
            FATAL_ERROR
            "gcov ${GCOV_EXECUTABLE} is major version ${CMAKE_MATCH_1}, but the compiler is "
            "major version ${_compiler_major}. Coverage data would not be read."
        )
    endif()

    # gcovr collects the gcov data into one HTML report. Without it there is no coverage target.
    find_program(GCOVR_EXECUTABLE gcovr)
    if(GCOVR_EXECUTABLE)
        # Python runs the per-site summary. Without it the coverage target still writes the report.
        find_package(Python3 COMPONENTS Interpreter)
        set(_coverage_python "")
        if(Python3_Interpreter_FOUND)
            set(_coverage_python "${Python3_EXECUTABLE}")
        endif()

        add_custom_target(
            coverage
            COMMAND
                "${CMAKE_COMMAND}"
                "-DCTEST_COMMAND=${CMAKE_CTEST_COMMAND}"
                "-DGCOVR=${GCOVR_EXECUTABLE}"
                "-DGCOV=${GCOV_EXECUTABLE}"
                "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
                "-DBINARY_DIR=${CMAKE_BINARY_DIR}"
                "-DPYTHON=${_coverage_python}"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/RunCoverage.cmake"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            VERBATIM
            USES_TERMINAL
            COMMENT "Running the tests and writing the coverage report"
        )
    else()
        message(
            WARNING
            "gcovr was not found, so the coverage target is not available"
        )
    endif()
endif()

# Profiling measurements use an optimized, unsanitized build regardless of
# the main build type so debug instrumentation does not distort timings.
add_library(hackystl_profile_config INTERFACE)
target_include_directories(
    hackystl_profile_config
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/lib"
        "${CMAKE_CURRENT_SOURCE_DIR}/Tests"
)
target_link_libraries(hackystl_profile_config INTERFACE Threads::Threads)
if(MSVC)
    target_compile_options(hackystl_profile_config INTERFACE /utf-8 /W4 /wd4530 /O2)
else()
    target_compile_options(
        hackystl_profile_config
        INTERFACE
            -fno-exceptions
            -Wall
            -Wextra
            -Wempty-body
            -Wignored-qualifiers
            -Wmissing-field-initializers
            -Wstring-compare
            -Wuninitialized
            -Wunused-parameter
            -O3
    )
endif()

if(BUILD_TESTING)
    file(
        GLOB_RECURSE HACKYSTL_TEST_SOURCES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/Tests/*.cpp"
    )

    # This file supplies symbols to defaultarg.cpp and is not an executable by
    # itself. It is added to that test explicitly below.
    list(
        REMOVE_ITEM
        HACKYSTL_TEST_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/Tests/DefaultArg/extern_impl.cpp"
    )

    foreach(test_source IN LISTS HACKYSTL_TEST_SOURCES)
        file(
            RELATIVE_PATH relative_test_source
            "${CMAKE_CURRENT_SOURCE_DIR}/Tests"
            "${test_source}"
        )
        string(REGEX REPLACE "\\.cpp$" "" test_name "${relative_test_source}")
        string(REPLACE "/" "_" test_name "${test_name}")
        set(test_target "hackystl_test_${test_name}")

        set(test_sources "${test_source}")
        if(relative_test_source STREQUAL "DefaultArg/defaultarg.cpp")
            list(
                APPEND test_sources
                "${CMAKE_CURRENT_SOURCE_DIR}/Tests/DefaultArg/extern_impl.cpp"
            )
        endif()

        add_executable("${test_target}" ${test_sources})
        if(relative_test_source MATCHES "^Profiling/")
            target_link_libraries(
                "${test_target}" PRIVATE hackystl_profile_config
            )
            # MSVC rejects /O2 together with the /RTC1 checks that Debug enables.
            set_target_properties(
                "${test_target}" PROPERTIES MSVC_RUNTIME_CHECKS ""
            )
        else()
            target_link_libraries("${test_target}" PRIVATE hackystl_test_config)
        endif()
        set_target_properties(
            "${test_target}"
            PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
        )

        get_filename_component(test_directory "${test_source}" DIRECTORY)
        set(test_working_directory "${test_directory}")

        if(relative_test_source STREQUAL "Io/IoTest.cpp")
            set(test_working_directory "${CMAKE_CURRENT_BINARY_DIR}/io_test")
            file(MAKE_DIRECTORY "${test_working_directory}")
            configure_file(
                "${CMAKE_CURRENT_SOURCE_DIR}/Tests/Io/test.txt"
                "${test_working_directory}/test.txt"
                COPYONLY
            )
            add_test(
                NAME "${test_target}"
                COMMAND
                    "${CMAKE_COMMAND}"
                    "-DPROGRAM=$<TARGET_FILE:${test_target}>"
                    "-DINPUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/Tests/Io/IoTest.input"
                    "-DFIXTURE_FILE=${CMAKE_CURRENT_SOURCE_DIR}/Tests/Io/test.txt"
                    "-DWORKING_DIRECTORY=${test_working_directory}"
                    "-DTIMEOUT=${HACKYSTL_INTERACTIVE_TIMEOUT}"
                    -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/RunWithInput.cmake"
            )
        else()
            add_test(NAME "${test_target}" COMMAND "${test_target}")
        endif()

        set_tests_properties(
            "${test_target}"
            PROPERTIES
                WORKING_DIRECTORY "${test_working_directory}"
                TIMEOUT "${HACKYSTL_INTERACTIVE_TIMEOUT}"
        )

        # Peer-dependent examples and timing-sensitive profiling workloads
        # are invoked manually. Protocol parsers remain automated.
        if(relative_test_source MATCHES "^Profiling/")
            set_tests_properties(
                "${test_target}"
                PROPERTIES DISABLED TRUE LABELS "manual;profiling"
            )
        elseif(
            relative_test_source MATCHES "^Network/HTTP/TestHttpServer\\.cpp$"
            OR relative_test_source MATCHES "^Network/(TCP|UDP)/"
        )
            set_tests_properties(
                "${test_target}"
                PROPERTIES DISABLED TRUE LABELS "manual"
            )
        else()
            if(relative_test_source MATCHES "^Problems/")
                set_tests_properties(
                    "${test_target}"
                    PROPERTIES LABELS "automated;problems"
                )
            else()
                set_tests_properties("${test_target}" PROPERTIES LABELS "automated")
            endif()
        endif()
    endforeach()
endif()

option(
    HACKYSTL_BUILD_EXAMPLES
    "Build the programs under Examples/ and register their output checks"
    ON
)

if(HACKYSTL_BUILD_EXAMPLES)
    file(
        GLOB_RECURSE HACKYSTL_EXAMPLE_SOURCES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/Examples/*.cpp"
    )

    find_package(Python3 COMPONENTS Interpreter)

    if(BUILD_TESTING)
        if(Python3_Interpreter_FOUND)
            add_test(
                NAME hackystl_examples_lint
                COMMAND
                    "${Python3_EXECUTABLE}"
                    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CheckExamples.py"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Examples"
            )
            set_tests_properties(
                hackystl_examples_lint PROPERTIES LABELS "examples"
            )
        else()
            message(
                STATUS
                "Python 3 not found: example output checks are not registered with CTest"
            )
        endif()
    endif()

    foreach(example_source IN LISTS HACKYSTL_EXAMPLE_SOURCES)
        file(
            RELATIVE_PATH relative_example_source
            "${CMAKE_CURRENT_SOURCE_DIR}/Examples"
            "${example_source}"
        )
        string(REGEX REPLACE "\\.cpp$" "" example_name "${relative_example_source}")
        string(REPLACE "/" "_" example_name "${example_name}")
        set(example_target "hackystl_example_${example_name}")

        add_executable("${example_target}" "${example_source}")
        target_link_libraries("${example_target}" PRIVATE hackystl_test_config)
        set_target_properties(
            "${example_target}"
            PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples"
        )

        if(NOT BUILD_TESTING)
            continue()
        endif()

        get_filename_component(example_directory "${example_source}" DIRECTORY)
        get_filename_component(example_stem "${example_source}" NAME_WE)
        set(example_cases "${example_directory}/${example_stem}.cases")

        # An example without a .cases file needs a peer or an interactive
        # session, so it is built and listed but never run automatically.
        if(NOT EXISTS "${example_cases}")
            add_test(NAME "${example_target}" COMMAND "${example_target}")
            set_tests_properties(
                "${example_target}"
                PROPERTIES DISABLED TRUE LABELS "examples;manual"
            )
        elseif(Python3_Interpreter_FOUND)
            add_test(
                NAME "${example_target}"
                COMMAND
                    "${Python3_EXECUTABLE}"
                    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/RunCases.py"
                    --program "$<TARGET_FILE:${example_target}>"
                    --cases "${example_cases}"
                    --fixtures "${example_directory}"
                    --workdir "${CMAKE_BINARY_DIR}/example_run/${example_name}"
                    --timeout "${HACKYSTL_INTERACTIVE_TIMEOUT}"
            )
            set_tests_properties(
                "${example_target}"
                PROPERTIES
                    LABELS "examples"
                    TIMEOUT "${HACKYSTL_INTERACTIVE_TIMEOUT}"
            )
        endif()
    endforeach()
endif()