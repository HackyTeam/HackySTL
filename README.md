<p align = 'left'>
  <img src='https://github.com/HackyTeam/HackySTL/blob/main/Docs/Logo.png?raw=true'>
</p>

This is a project which discards any backwards compatibility
thus, it's written using the latest version of the C++ standard.

# Contributors:
- DeKrain (Implementations)
- qookie (Mostly debugging and some implementations)
- Claude and GPT for documentation testing and bug fixes (See AI use policy)

# Building and testing

The repository uses CMake to build the standalone test programs without
changing the existing source hierarchy. The default configuration uses C++23,
the repository warning flags, `-fno-exceptions`, pthreads, and
AddressSanitizer/UndefinedBehaviorSanitizer when supported.

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Sanitizers can be disabled for a faster build with
`-DHACKYSTL_ENABLE_SANITIZERS=OFF`. Peer-dependent HTTP/TCP/UDP examples are
built but disabled in CTest; self-contained local protocol tests still run
automatically. `IoTest` is run with `Tests/Io/IoTest.input` as stdin and uses a
copied fixture under `build/`, keeping the source tree unchanged.

For a socket example, start its matching server in one terminal and run the
client from another. Piped client input is also supported, for example:

```sh
printf 'hello\n' | build/tests/hackystl_test_Network_TCP_TClientTest
```

The interactive CTest timeout can be changed with
`-DHACKYSTL_INTERACTIVE_TIMEOUT=30`.

## Testing with sanitizers

On GCC and Clang, the test executables are built with AddressSanitizer and
UndefinedBehaviorSanitizer. The option is `ON` by default, so the commands
above already run the whole suite under both sanitizers. Pass the option
explicitly to make the intent visible in a script:

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DHACKYSTL_ENABLE_SANITIZERS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

A sanitizer report makes its test fail, and the report appears in the
`--output-on-failure` output. Undefined-behavior reports do not include a stack
trace by default. Set `UBSAN_OPTIONS=print_stacktrace=1` to get one, either when
running a test directly or through CTest:

```sh
UBSAN_OPTIONS=print_stacktrace=1 build/tests/hackystl_test_Vector_VecTest
UBSAN_OPTIONS=print_stacktrace=1 ctest --test-dir build -R '^hackystl_test_Vector_VecTest$' --output-on-failure
```

Two cases do not use the sanitizers. `LibraryProfile` always builds optimized
without them, so its timings stay meaningful. MSVC builds ignore the option,
because the CMake configuration only adds the sanitizer flags for GCC, Clang,
and AppleClang.

## Testing with Clang

Clang is selected with CMake's compiler variable. Use a separate build
directory for each compiler, because CMake records the compiler in the build
directory it configures:

```sh
cmake -S . -B build-clang -G Ninja -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug
cmake --build build-clang --parallel
ctest --test-dir build-clang --output-on-failure
```

The Clang build uses the same sanitizer flags as the GCC build, so this run is
also sanitized. The `build` and `build-clang` directories are ignored by git.

## Profiling the library

The library benchmark is the `hackystl_test_Profiling_LibraryProfile` target.
CTest registers it as disabled, so `ctest` skips it. Build and run it directly:

```sh
cmake --build build --target hackystl_test_Profiling_LibraryProfile
./build/tests/hackystl_test_Profiling_LibraryProfile
```

The benchmark always builds with `-O3` and without sanitizers, whatever the
build type, so its timings are not inflated by instrumentation. Each output line
is one workload:

```
unordered_map insert: HackySTL=9.47 ns/op [9.25, 11.15], std=14.69 ns/op [14.54, 15.56], checksum=3539803
```

The first number for each implementation is the median time per operation, after
two warmups and eleven measured samples. The bracketed range is the fastest and
slowest sample. Workloads with a standard-library counterpart run both
implementations in alternating order, and the program asserts that their
checksums match. Timings depend on the machine, so compare runs taken on the same
machine. The range shows run-to-run spread, not a confidence interval.
Filesystem and network workloads are not measured. The full workload list is in
[Tests/README.md](Tests/README.md#profiling).

## Tracing your own code

`Serialize/Tracing.hpp` times scopes in your own code. Put `lib/` on the include
path, and define `HSD_ENABLE_TRACING` before including the header or pass
`-DHSD_ENABLE_TRACING` to the compiler. Without it, `HSD_TRACE_SCOPE()` expands
to nothing:

```cpp
#include <Serialize/Tracing.hpp>

void load_items()
{
    HSD_TRACE_SCOPE();   // timed until the end of this block
    // ...
}

int main()
{
    load_items();
    hsd::trace_sink::flush();   // writes the buffered lines to stdout
}
```

Each finished scope prints one line with its function, file and line, its
nesting depth on the thread, its start time in monotonic nanoseconds, and its
duration:

```
load_items src/items.cpp:9 depth 1 start 27330356061689ns took 20ns
```

Output stays in a buffer until `hsd::trace_sink::flush()` is called, so events
still buffered when the program exits are lost. Events from different threads
are serialized, and nesting depth is tracked per thread. To send events
somewhere other than standard output, pass a function taking
`const hsd::trace_event&` to `hsd::trace_sink::set_hook`. A hook must not open a
`trace_scope` of its own.

# Examples

[Examples/](Examples/) holds about 150 small programs, three per feature at
increasing complexity (Basic, Intermediate, Advanced), laid out like `Tests/`.
Each program prints its results, and CTest compares that output with a
`.cases` file beside it, using the Moodle VPL syntax. The target list and the
case format are described in [Examples/README.md](Examples/README.md).

```sh
cmake --build build --target hackystl_example_Algorithm_SortBasics
ctest --test-dir build -L examples --output-on-failure
```

Pass `-DHACKYSTL_BUILD_EXAMPLES=OFF` to skip building them.

# Documentation

The complete library header catalog is maintained in
[Docs/LibraryReference.md](Docs/LibraryReference.md). It covers every header
under `lib/`, identifies public versus implementation-detail modules, and
describes each header's role and ownership boundaries.

Generate the HTML documentation with:

```sh
cd Docs && doxygen Doxyfile
```

Open [Docs/html/index.html](Docs/html/index.html) after generation. Run the
command from `Docs/` with `Doxyfile`; `doxygen .` does not select the project
configuration or generate the expected output tree.

one range pipeline, and string mutation/content synchronization. Peer-based
The container and utility tests include differential checks against the
standard library where the APIs are comparable. They cover repeated mutation,
copying/moving, boundary access, error paths, object lifetime, allocator-backed
storage, and constexpr behavior. A test-by-test coverage catalog, generated
target naming, fixture details, and manual network examples are documented in
[Tests/README.md](Tests/README.md).

# AI use policy

HackySTL represented by its project founder and main developer reserves to note 
that AI use in contributions are allowed as long as they meet the following criterion:
- The code is simple and understandable by all of the library contributors
- The output has been reviewed and verified by a non-agent
- Ussage of AI has been disclosed
- The output has been explained thoroughly
