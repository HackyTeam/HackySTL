cmake_minimum_required(VERSION 3.23)

project(HackySTL LANGUAGES CXX)

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
    target_compile_options(hackystl_test_config INTERFACE /W4 /EHs-c /UNDEBUG)
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
    target_compile_options(hackystl_profile_config INTERFACE /W4 /EHs-c /O2)
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