# Test Suite

CTest discovers each test executable from a `.cpp` file under `Tests/`. The
target name is `hackystl_test_` followed by the source path relative to `Tests/`,
with `/` replaced by `_` and `.cpp` removed. The current suite has 130 test targets and 156 example targets. `DefaultArg/extern_impl.cpp` is linked into `DefaultArg/defaultarg.cpp`
as support code and is not a separate test target.

## Build and Run

From the repository root:

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Run one target by building it and selecting its exact CTest name:

```sh
cmake --build build --target hackystl_test_Problems_RoutePlannerTest
ctest --test-dir build -R '^hackystl_test_Problems_RoutePlannerTest$' --output-on-failure
```

Problem scenarios also have the `problems` label and can be selected with
`ctest --test-dir build -L problems --output-on-failure`.

Sanitizer runs and Clang builds are described in the
[root README](../README.md#testing-with-sanitizers).

## Profiling

The profiling executable reports median nanoseconds per operation and the
sample range after two warmups and eleven measurements. 

Paired HackySTL and standard-library workloads alternate which implementation 
runs first, reducing systematic order bias. 

The map, deque, and string-view benchmarks use runtime-seeded inputs and separate 
insertion, lookup, traversal, hit, and miss cases. 

A pre-reserved HackySTL deque insertion case distinguishes capacity
growth from per-element insertion cost; map insertion is also measured with
both implementations pre-reserved. 

Prepared-state measurements exclude container setup and teardown from the timed 
insertion loop; lookup and traversal fixtures are initialized during warmup. 

Scalar checksum constraints in remaining workloads keep loop results observable 
without a memory clobber on every iteration; ownership workloads also preserve 
allocations so the compiler cannot elide standard-library new/delete pairs. 

It covers dynamic and fixed containers (vector, deque, list, forward-list, 
string, unordered map, arrays, bitset, span, and string view), sorting and 
range pipelines, type-erased and ownership wrappers, tuple/variant access, 
random generation, math, JSON parsing, and Unicode conversion.

Operations with standard-library counterparts run side by side and verify
matching checksums; HackySTL-only operations report their own checksums.

Timings are informational, not pass/fail thresholds, and should be compared
using the same machine. The sample range shows observed run-to-run spread but
is not a confidence interval; CPU scheduling, frequency scaling, 
and other system activity still affect results. 

`ns/op` is normalized by each workload's declared count, which can be an 
element or iteration unit rather than one primitive operation; compare the 
paired implementations within each workload.

This target always uses optimization (`-O3`, or `/O2` with MSVC) and omits 
sanitizers, regardless of the main build type.

Filesystem/network timings are omitted because they depend on external state
and services; compile-time-only features have no runtime workload. Profiling
tests are excluded from normal CTest runs.

```sh
cmake --build build --target hackystl_test_Profiling_LibraryProfile
./build/tests/hackystl_test_Profiling_LibraryProfile
```

## Fixtures and Manual Examples

`Io/IoTest.cpp` runs through a CMake wrapper: `Io/IoTest.input` is supplied as
stdin, `Io/test.txt` is copied to `build/io_test/`, and the test runs from that
build directory. The source fixtures remain unchanged.

The HTTP server and TCP/UDP client/server examples are registered but disabled
in CTest because they require an interactive peer. Profiling tests are also
excluded from CTest and are run directly as shown above. Start a matching
server before running a client example manually. `Problems/UdpLoopbackTest.cpp`
is a self-contained localhost exchange and runs automatically.

## Test Inventory

### Allocation and Containers

| CTest target | Source | Coverage |
| --- | --- | --- |
| `hackystl_test_Allocator_AllocatorTest` | [Allocator/AllocatorTest.cpp](Allocator/AllocatorTest.cpp) | Allocation alignment and failure paths, buffered allocation/coalescing, stateful allocator propagation, and construction/destruction accounting. |
| `hackystl_test_Any_AnyTest` | [Any/AnyTest.cpp](Any/AnyTest.cpp) | Empty and populated type-erased values compared with `std::any`, including type checks, emplace, copy/move independence, and reset. |
| `hackystl_test_Any_AnyCastTest` | [Any/AnyCastTest.cpp](Any/AnyCastTest.cpp) | Mutable casts on a non-const `any`, and read-only casts on a const `any`, checked by their return types and by reads and writes. |
| `hackystl_test_Any_AnyStorageTest` | [Any/AnyStorageTest.cpp](Any/AnyStorageTest.cpp) | Inline and heap-held values: destruction counts, address stability when a heap value moves, copies that are independent objects, swap across storage kinds, and `emplace` built from the old value. |
| `hackystl_test_Any_AnyCopyTest` | [Any/AnyCopyTest.cpp](Any/AnyCopyTest.cpp) | Copying, moving, and swapping empty, small, and large values, including copies of an empty `any`. |
| `hackystl_test_Array_Heap_HeapArrTest` | [Array/Heap/HeapArrTest.cpp](Array/Heap/HeapArrTest.cpp) | Fixed-size heap array access, bounds errors, mutation, deep copies, and moves. |
| `hackystl_test_Array_Stack_StackArrTest` | [Array/Stack/StackArrTest.cpp](Array/Stack/StackArrTest.cpp) | Fixed-size stack array sequence, fill, checked access, assignment, and swap. |
| `hackystl_test_Bitset_TestBitset` | [Bitset/TestBitset.cpp](Bitset/TestBitset.cpp) | Whole-set and indexed bit operations compared with `std::bitset`, bounds errors, shifts, counting, and constexpr behavior. |
| `hackystl_test_Deque_TestDeque` | [Deque/TestDeque.cpp](Deque/TestDeque.cpp) | Front/back insertion and removal, resize, erase, and special members compared with `std::deque`. |
| `hackystl_test_Deque_DequeGrowthTest` | [Deque/DequeGrowthTest.cpp](Deque/DequeGrowthTest.cpp) | Capacity growth with wrapped ring contents for trivially copyable and owning elements, plus reserve, resize, and copies, compared with `std::deque`. |
| `hackystl_test_Filesystem_FSTest` | [Filesystem/FSTest.cpp](Filesystem/FSTest.cpp) | Path status, directory listing/reuse, parent traversal, extension lookup, and path copy/move behavior. |
| `hackystl_test_List_Double_DListTest` | [List/Double/DListTest.cpp](List/Double/DListTest.cpp) | Doubly linked list mutations and edge cases compared with `std::list`, including copy, move, erase, and clear. |
| `hackystl_test_List_Single_SListTest` | [List/Single/SListTest.cpp](List/Single/SListTest.cpp) | Forward-list insertion, removal, erase, and special members compared with `std::forward_list`. |
| `hackystl_test_UnorderedMap_UMapTest` | [UnorderedMap/UMapTest.cpp](UnorderedMap/UMapTest.cpp) | Insert/update/erase/lookup and special members compared with `std::unordered_map`, including missing and duplicate keys. |
| `hackystl_test_UnorderedMap_UMapStressTest` | [UnorderedMap/UMapStressTest.cpp](UnorderedMap/UMapStressTest.cpp) | Randomized insert, lookup, erase, clear, reserve, copy, and move sequences compared with `std::unordered_map`, including all-colliding, identity, high-bit-only, and string-key hashing. |
| `hackystl_test_Vector_VecTest` | [Vector/VecTest.cpp](Vector/VecTest.cpp) | Repeated vector operations compared with `std::vector`, object lifetimes, bounds errors, special members, and constexpr storage. |
| `hackystl_test_Vector_VecGrowthTest` | [Vector/VecGrowthTest.cpp](Vector/VecGrowthTest.cpp) | Randomized growth, resize, shrink, clear, and copy sequences compared with `std::vector` for trivially copyable and owning elements, with live-object accounting. |
| `hackystl_test_Vector_VecCapacityGrowthTest` | [Vector/VecCapacityGrowthTest.cpp](Vector/VecCapacityGrowthTest.cpp) | Capacity doubles on push, and `reserve` and `resize` round up to the first power of two large enough. |
| `hackystl_test_Utils_CompressedPair` | [Utils/CompressedPair.cpp](Utils/CompressedPair.cpp) | Compressed-pair element access, mutation, and empty-type storage layout. |
| `hackystl_test_Utils_TypeTraitsUtilityTest` | [Utils/TypeTraitsUtilityTest.cpp](Utils/TypeTraitsUtilityTest.cpp) | `is_nothrow_constructible` against `noexcept` and throwing constructors, and `launder` in constant evaluation and after a replaced object. |
| `hackystl_test_Utils_ConceptsTest` | [Utils/ConceptsTest.cpp](Utils/ConceptsTest.cpp) | Every concept in `Concepts.hpp` checked at compile time against types it accepts and rejects, including the container and iterator concepts. |
| `hackystl_test_Utils_RawStorageTest` | [Utils/RawStorageTest.cpp](Utils/RawStorageTest.cpp) | `construct_at` in constant evaluation, and `new_allocate`/`delete_deallocate` alignment from 8 through 4096 bytes. |
| `hackystl_test_Utils_BitScanTest` | [Utils/BitScanTest.cpp](Utils/BitScanTest.cpp) | `countl_zero` and `countr_zero` against `std::countl_zero`/`std::countr_zero` for every width from 8 to 64 bits, at powers of two, their neighbours, zero, all ones, and pseudo-random values; constant-evaluation checks. |
| `hackystl_test_Utils_Int128Test` | [Utils/Int128Test.cpp](Utils/Int128Test.cpp) | The library's 128-bit integer (`_Int128.hpp`): wrap-around, carries and borrows, multiplication, division by words and by larger divisors, shifts, signed division and shifts, float conversions, and formatting where it is the `u128` alias. |

### Values, Parsing, and Formatting

| CTest target | Source | Coverage |
| --- | --- | --- |
| `hackystl_test_ArgParse_ArgParseTest` | [ArgParse/ArgParseTest.cpp](ArgParse/ArgParseTest.cpp) | Registered option dispatch, integer/string callbacks, repeated parsing, hexadecimal input, and parser-stream failures. |
| `hackystl_test_DefaultArg_defaultarg` | [DefaultArg/defaultarg.cpp](DefaultArg/defaultarg.cpp) | Default-call arguments when omitted, partially overridden, or fully supplied. |
| `hackystl_test_Io_IoTest` | [Io/IoTest.cpp](Io/IoTest.cpp) | Formatting overrides, stream input conversion, file fixture parsing, and formatted file output. |
| `hackystl_test_Io_StdStreamTest` | [Io/StdStreamTest.cpp](Io/StdStreamTest.cpp) | Standard output and error descriptors stay open after `io` output, checked during static destruction. |
| `hackystl_test_Json_JsonTest` | [Json/JsonTest.cpp](Json/JsonTest.cpp) | JSON scalar/object/array parsing, typed access and errors, numeric boundaries, chunked lexing, nested values, and malformed input. |
| `hackystl_test_Json_JsonTestFile` | [Json/JsonTestFile.cpp](Json/JsonTestFile.cpp) | File-based JSON lexing and nested fixture validation across strings, numbers, arrays, objects, and primitives. |
| `hackystl_test_String_StrTest` | [String/StrTest.cpp](String/StrTest.cpp) | String mutation/search/substrings and special members, with randomized operation sequences compared against `std::string`. |
| `hackystl_test_String_StringViewFindTest` | [String/StringViewFindTest.cpp](String/StringViewFindTest.cpp) | String-view substring and `char` search from every start position compared with `std::string_view`, over small alphabets, embedded nulls, high-bit bytes, wide characters, and at compile time. |
| `hackystl_test_String_StringViewNulTest` | [String/StringViewNulTest.cpp](String/StringViewNulTest.cpp) | Embedded nulls in `==`, `<=>`, `find`, and `rfind`, checked at compile time and at run time, and ordering compared with `std::string_view`. |
| `hackystl_test_String_StringViewAffixTest` | [String/StringViewAffixTest.cpp](String/StringViewAffixTest.cpp) | `starts_with` and `ends_with` for characters and views, including empty views, needles longer than the view, and zero-length views over longer text. |
| `hackystl_test_String_StringNulTest` | [String/StringNulTest.cpp](String/StringNulTest.cpp) | `hsd::string` with embedded nulls: construction, copy, `+`, `+=`, `==`, `<=>`, `find`, and `rfind`, against strings and views, plus random comparisons with `std::string` that include bytes above `0x7f`. |
| `hackystl_test_String_StringViewRfindTest` | [String/StringViewRfindTest.cpp](String/StringViewRfindTest.cpp) | `rfind` for views: empty and repeated needles, bounded searches that must not read past the view, start positions past the end, and random comparisons with `std::string_view`. |
| `hackystl_test_StringParser_StringParserTest` | [StringParser/StringParserTest.cpp](StringParser/StringParserTest.cpp) | Compile-time format matching, typed integer/float conversion, string captures, and format mismatch errors. |
| `hackystl_test_ToString_IntegralToStringTest` | [ToString/IntegralToStringTest.cpp](ToString/IntegralToStringTest.cpp) | Decimal and hexadecimal integer text for every integral width, compared with `printf`, with literal checks for 128-bit values, wide character output, and constant evaluation. |
| `hackystl_test_ToString_HexFloatToStringTest` | [ToString/HexFloatToStringTest.cpp](ToString/HexFloatToStringTest.cpp) | Hexadecimal float text for `float`, `double`, and `long double`: exact strings for known values, random values parsed back with the C library, and constant evaluation. |
| `hackystl_test_ToString_StringConversionTest` | [ToString/StringConversionTest.cpp](ToString/StringConversionTest.cpp) | `hsd::string` numeric conversions (`to_string` and the wide variants) keep every character and return the string passed to them. |
| `hackystl_test_ToString_FloatToStringTest` | [ToString/FloatToStringTest.cpp](ToString/FloatToStringTest.cpp) | Fixed and scientific float text compared with `printf` for random `float`, `double`, and `long double` bit patterns, precisions on both sides of the 128-bit path, ties and carries, extremes, and constant evaluation. |
| `hackystl_test_Unicode_UniTest` | [Unicode/UniTest.cpp](Unicode/UniTest.cpp) | UTF-8/UTF-16/UTF-32 conversions and insufficient-output-buffer failure. |
| `hackystl_test_Logger_LogTest` | [Logger/LogTest.cpp](Logger/LogTest.cpp) | Stack-trace capture, default handles that own nothing, per-thread stack isolation, non-copyable handles, scope timing, and hexadecimal floating-point formatting. |
| `hackystl_test_Result_ResTest` | [Result/ResTest.cpp](Result/ResTest.cpp) | Success/error results and optional values compared with `std::expected`/`std::optional`, plus fallback recovery and move extraction. |
| `hackystl_test_Switch_SwitchTest` | [Switch/SwitchTest.cpp](Switch/SwitchTest.cpp) | Equality, ordering, default dispatch, and callback selection for string and numeric values. |
| `hackystl_test_Variant_VarTest` | [Variant/VarTest.cpp](Variant/VarTest.cpp) | Variant type/index access, copy/move, and visitors compared with `std::variant`, plus checked failures, in-place construction, and const access. |

### Concurrency and Ownership

| CTest target | Source | Coverage |
| --- | --- | --- |
| `hackystl_test_Atomic_AtTest` | [Atomic/AtTest.cpp](Atomic/AtTest.cpp) | Atomic exchange, compare/exchange success and failure, and bitwise fetch operations compared with `std::atomic`, plus contended updates with threads. |
| `hackystl_test_CondVar_CondVarTest` | [CondVar/CondVarTest.cpp](CondVar/CondVarTest.cpp) | Predicate-based condition-variable waits, notification, and shared-state protection compared with `std::condition_variable`. |
| `hackystl_test_CondVar_CondVarWaitForTest` | [CondVar/CondVarWaitForTest.cpp](CondVar/CondVarWaitForTest.cpp) | Relative waits with no notifier last their full duration, both with and without a predicate, measured on the monotonic clock. |
| `hackystl_test_CondVar_CondVarTimeoutTest` | [CondVar/CondVarTimeoutTest.cpp](CondVar/CondVarTimeoutTest.cpp) | Conversion of absolute deadlines to native relative millisecond timeouts: exact values, round-up, past deadlines, and the cap. |
| `hackystl_test_Tracing_TracingTest` | [Tracing/TracingTest.cpp](Tracing/TracingTest.cpp) | Scope nesting depth and duration, the `HSD_TRACE_SCOPE` macro, buffered output until `flush`, one line per event, non-copyable scopes, and per-thread depth under four concurrent threads. |
| `hackystl_test_Functional_FuncTest` | [Functional/FuncTest.cpp](Functional/FuncTest.cpp) | Type-erased callable behavior compared with `std::function`, including stateful, copied, moved, large, void, and result-returning callables; empty invocation and packaged-task results compared with `std::packaged_task`. |
| `hackystl_test_Functional_FuncLifetimeTest` | [Functional/FuncLifetimeTest.cpp](Functional/FuncLifetimeTest.cpp) | Target lifetimes through null assignment, move assignment, small-into-heap moves, swap across storage kinds, and self-assignment, counted so leaks and double destruction show up. |
| `hackystl_test_Future_FutureTest` | [Future/FutureTest.cpp](Future/FutureTest.cpp) | Promise and packaged-task futures carrying values, errors, and references across threads, with promise values checked against `std::promise`/`std::future`. |
| `hackystl_test_Lock_LockTest` | [Lock/LockTest.cpp](Lock/LockTest.cpp) | Futex, mutex, and spin-lock synchronization while worker threads update shared maps. |
| `hackystl_test_SharedPtr_ShrPtrTest` | [SharedPtr/ShrPtrTest.cpp](SharedPtr/ShrPtrTest.cpp) | Atomic and non-atomic shared ownership compared with `std::shared_ptr`, conversions, arrays, custom allocators, lifetime, and pointer comparisons. |
| `hackystl_test_SharedPtr_SharedPtrBlockTest` | [SharedPtr/SharedPtrBlockTest.cpp](SharedPtr/SharedPtrBlockTest.cpp) | `make_shared` allocation count and block-size contract, over-aligned types, derived-to-base conversion, empty owners, self-assignment, and a randomized comparison with `std::shared_ptr` for atomic and non-atomic owners. |
| `hackystl_test_SharedPtr_ThreadedPtrTest` | [SharedPtr/ThreadedPtrTest.cpp](SharedPtr/ThreadedPtrTest.cpp) | Multiple threads reading shared ownership while synchronized by a spin lock. |
| `hackystl_test_SharedPtr_SharedPtrReleaseRaceTest` | [SharedPtr/SharedPtrReleaseRaceTest.cpp](SharedPtr/SharedPtrReleaseRaceTest.cpp) | Two threads releasing the last two atomic references to each object at the same instant; every object is destroyed and freed exactly once. |
| `hackystl_test_Thread_ThreadTest` | [Thread/ThreadTest.cpp](Thread/ThreadTest.cpp) | Bound member invocation and move construction/assignment compared with `std::thread`, joinability, joining, and invalid repeated join. |
| `hackystl_test_UniquePtr_UniqueTest` | [UniquePtr/UniqueTest.cpp](UniquePtr/UniqueTest.cpp) | Unique ownership compared with `std::unique_ptr` for objects, arrays, moves, reset, and polymorphic destruction, plus buffered allocation. |

### Algorithms, Ranges, and Numerics

| CTest target | Source | Coverage |
| --- | --- | --- |
| `hackystl_test_Gorbitsa_GorTest` | [Gorbitsa/GorTest.cpp](Gorbitsa/GorTest.cpp) | Compile-time bytecode parsing/execution and exact emitted output. |
| `hackystl_test_Integer_TestInteger` | [Integer/TestInteger.cpp](Integer/TestInteger.cpp) | Multi-precision addition, subtraction, multiplication, quotient/remainder, zero-divisor errors, and wide-width wraparound. |
| `hackystl_test_IntegerSequence_IntegerSeqTest` | [IntegerSequence/IntegerSeqTest.cpp](IntegerSequence/IntegerSeqTest.cpp) | Compile-time integer/index sequence generation, ranges, inversion, and pack expansion. |
| `hackystl_test_Limits_LimTest` | [Limits/LimTest.cpp](Limits/LimTest.cpp) | Integral and floating-point boundaries, signedness, epsilon, and denormal values compared with `std::numeric_limits`. |
| `hackystl_test_Math_MathTest` | [Math/MathTest.cpp](Math/MathTest.cpp) | Elementary, trigonometric, hyperbolic, logarithmic, and gamma functions compared with the standard library. |
| `hackystl_test_Random_RandTest` | [Random/RandTest.cpp](Random/RandTest.cpp) | Integer/floating generation bounds, invalid ranges, seeded reproducibility, and raw engine output compared with `std::mt19937`. |
| `hackystl_test_Ranges_EnumerateTest` | [Ranges/EnumerateTest.cpp](Ranges/EnumerateTest.cpp) | Enumerated indices and mutable elements compared with `std::views::enumerate`, plus empty-range and fresh-iteration behavior. |
| `hackystl_test_Ranges_RangeTest` | [Ranges/RangeTest.cpp](Ranges/RangeTest.cpp) | Drop/take and predicate views, filtering/transformation pipelines, reverse traversal, and filtered mutation compared with standard range views. |
| `hackystl_test_Ranges_TrimTest` | [Ranges/TrimTest.cpp](Ranges/TrimTest.cpp) | Whitespace trimming and predicate-based dropping at range boundaries. |
| `hackystl_test_Algorithm_QuickSortTest` | [Algorithm/QuickSortTest.cpp](Algorithm/QuickSortTest.cpp) | Sorted, reversed, constant, few-distinct, organ-pipe, and sawtooth inputs compared with `std::sort`, empty and tiny containers, predicates, owning elements, and compile-time sorting. |
| `hackystl_test_Time_ClockTest` | [Time/ClockTest.cpp](Time/ClockTest.cpp) | Precise/process clock arithmetic and duration literals compared with `std::chrono`, plus a runtime clock measurement. |
| `hackystl_test_Time_PreciseClockTest` | [Time/PreciseClockTest.cpp](Time/PreciseClockTest.cpp) | `precise_clock::now()` agrees with the C library's wall time, and a 50 ms sleep is visible in its differences. |
| `hackystl_test_Time_MonotonicClockTest` | [Time/MonotonicClockTest.cpp](Time/MonotonicClockTest.cpp) | Monotonic readings never decrease over 10,000 samples, and a 20 ms sleep is visible in the elapsed time. |
| `hackystl_test_Time_DateTest` | [Time/DateTest.cpp](Time/DateTest.cpp) | Current date refresh, text formatting, date literals, calendar fields, and comparisons. |
| `hackystl_test_Tuple_FizzBuzz` | [Tuple/FizzBuzz.cpp](Tuple/FizzBuzz.cpp) | FizzBuzz classification for representative inputs and the range 1 through 1000. |
| `hackystl_test_Tuple_TupTest` | [Tuple/TupTest.cpp](Tuple/TupTest.cpp) | Tuple construction/access, concatenation, reference ties, and copying compared with `std::tuple`. |

### Profiling

| CTest target | Source | Coverage |
| --- | --- | --- |
| `hackystl_test_Profiling_LibraryProfile` | [Profiling/LibraryProfile.cpp](Profiling/LibraryProfile.cpp) | Optimized informational timings across containers, algorithms, ranges, wrappers, and parsing; paired workloads verify output-checksum parity and run manually. |

### Network Protocols

| CTest target | Source | Coverage |
| --- | --- | --- |
| `hackystl_test_Network_HTTP_TestHttpHeader` | [Network/HTTP/TestHttpHeader.cpp](Network/HTTP/TestHttpHeader.cpp) | CRLF header parsing, case-insensitive lookup and overwrite, missing fields, clear, and malformed input. |
| `hackystl_test_Network_HTTP_TestHttpRequest` | [Network/HTTP/TestHttpRequest.cpp](Network/HTTP/TestHttpRequest.cpp) | HTTP request parsing, method/path/version/header access, request builders, clear behavior, and malformed line endings. |
| `hackystl_test_Network_HTTP_TestHttpResponse` | [Network/HTTP/TestHttpResponse.cpp](Network/HTTP/TestHttpResponse.cpp) | HTTP response parsing, status/reason/header access, response builders, and malformed line endings. |
| `hackystl_test_Network_HTTP_TestHttpServer` | [Network/HTTP/TestHttpServer.cpp](Network/HTTP/TestHttpServer.cpp) | Manual interactive HTTP server example; disabled in CTest because it requires a client. |
| `hackystl_test_Network_TCP_TClientTest` | [Network/TCP/TClientTest.cpp](Network/TCP/TClientTest.cpp) | Manual TCP client example; disabled in CTest because it requires a server. |
| `hackystl_test_Network_TCP_TServerTest` | [Network/TCP/TServerTest.cpp](Network/TCP/TServerTest.cpp) | Manual TCP server example; disabled in CTest because it requires a client. |
| `hackystl_test_Network_UDP_UClientTest` | [Network/UDP/UClientTest.cpp](Network/UDP/UClientTest.cpp) | Manual UDP client example; disabled in CTest because it requires a server. |
| `hackystl_test_Network_UDP_UServerTest` | [Network/UDP/UServerTest.cpp](Network/UDP/UServerTest.cpp) | Manual UDP server example; disabled in CTest because it requires a client. |

### Problem Scenarios

| CTest target | Source | Coverage |
| --- | --- | --- |
| `hackystl_test_Problems_DependencyPlannerTest` | [Problems/DependencyPlannerTest.cpp](Problems/DependencyPlannerTest.cpp) | Topological task planning with prerequisite ordering, cycle detection, and invalid task references. |
| `hackystl_test_Problems_ExpenseLedgerTest` | [Problems/ExpenseLedgerTest.cpp](Problems/ExpenseLedgerTest.cpp) | Parsed deposits, withdrawals, and transfers; balances and filtered ledger totals. |
| `hackystl_test_Problems_HeightMapPathTest` | [Problems/HeightMapPathTest.cpp](Problems/HeightMapPathTest.cpp) | Elevation-constrained shortest path compared with a standard-container reference search. |
| `hackystl_test_Problems_HttpJsonGatewayTest` | [Problems/HttpJsonGatewayTest.cpp](Problems/HttpJsonGatewayTest.cpp) | HTTP POST/201 exchange with JSON request and response bodies. |
| `hackystl_test_Problems_InventoryReplayTest` | [Problems/InventoryReplayTest.cpp](Problems/InventoryReplayTest.cpp) | Chunked JSON order replay with repeated line aggregation, stock validation, duplicate IDs, and revenue totals. |
| `hackystl_test_Problems_JsonChecksumTest` | [Problems/JsonChecksumTest.cpp](Problems/JsonChecksumTest.cpp) | Recursive JSON traversal with numeric/string/depth metrics and nested typed access. |
| `hackystl_test_Problems_LifeSimulationTest` | [Problems/LifeSimulationTest.cpp](Problems/LifeSimulationTest.cpp) | Bitset-backed Game of Life still-life and period-two oscillator behavior. |
| `hackystl_test_Problems_LogPipelineTest` | [Problems/LogPipelineTest.cpp](Problems/LogPipelineTest.cpp) | Structured log parsing, severity/service aggregation, slow-request filtering, and malformed records. |
| `hackystl_test_Problems_LruCacheTest` | [Problems/LruCacheTest.cpp](Problems/LruCacheTest.cpp) | List/map-backed LRU recency promotion, updates, misses, and capacity eviction. |
| `hackystl_test_Problems_PortfolioReconciliationTest` | [Problems/PortfolioReconciliationTest.cpp](Problems/PortfolioReconciliationTest.cpp) | Trade validation, position/cash updates, cost basis, JSON quotes, and realized/unrealized P&L reconciliation. |
| `hackystl_test_Problems_RoutePlannerTest` | [Problems/RoutePlannerTest.cpp](Problems/RoutePlannerTest.cpp) | Dijkstra route costs and reconstruction, same-node paths, unreachable nodes, and negative costs. |
| `hackystl_test_Problems_SlidingWindowMaximumTest` | [Problems/SlidingWindowMaximumTest.cpp](Problems/SlidingWindowMaximumTest.cpp) | Monotonic-deque sliding maxima compared with a standard-library reference across window edge cases. |
| `hackystl_test_Problems_TicTacToeReplayTest` | [Problems/TicTacToeReplayTest.cpp](Problems/TicTacToeReplayTest.cpp) | Bitset game replay with turn/occupancy validation, win detection, and draw detection. |
| `hackystl_test_Problems_UdpLoopbackTest` | [Problems/UdpLoopbackTest.cpp](Problems/UdpLoopbackTest.cpp) | Self-contained UDP request/reply over localhost using an ephemeral port. |
| `hackystl_test_Problems_WordIndexTest` | [Problems/WordIndexTest.cpp](Problems/WordIndexTest.cpp) | Case-insensitive positional index, multi-term ranking, phrase matching, and filtered hits. |
| `hackystl_test_Problems_WordLadderTest` | [Problems/WordLadderTest.cpp](Problems/WordLadderTest.cpp) | Word-ladder BFS compared with a standard-container reference for reachable, unreachable, and identical endpoints. |

### Regression Tests

Tests added with the fixes for the edge cases found while writing examples. Each
file fails against the code it covers before the fix, except `RandRangeEdgeTest`,
which checks bounds only: the upper-edge rounding it guards against occurs too
rarely to sample.

| CTest target | Source | Coverage |
| --- | --- | --- |
| `hackystl_test_Unicode_UniEdgeTest` | [Unicode/UniEdgeTest.cpp](Unicode/UniEdgeTest.cpp) | Every pair of UTF-8, UTF-16 and UTF-32 unit widths against an independent encoder, with exact, bounded and too-small destinations, empty and malformed input, and a guard region past each destination. |
| `hackystl_test_String_StringConcatEdgeTest` | [String/StringConcatEdgeTest.cpp](String/StringConcatEdgeTest.cpp) | Concatenation with empty and unallocated operands, appends into reserved capacity, self-append and aliasing, and long appends compared with `std::string`. |
| `hackystl_test_String_StringViewRfindEdgeTest` | [String/StringViewRfindEdgeTest.cpp](String/StringViewRfindEdgeTest.cpp) | Single-character reverse search in constant evaluation and on empty views. |
| `hackystl_test_String_StringViewPrintEdgeTest` | [String/StringViewPrintEdgeTest.cpp](String/StringViewPrintEdgeTest.cpp) | An empty string view prints as nothing and does not read its null data pointer. |
| `hackystl_test_Io_IoLineEdgeTest` | [Io/IoLineEdgeTest.cpp](Io/IoLineEdgeTest.cpp) | Line reading keeps the final line without a newline, blank lines, CRLF endings, and lines longer than the first buffer; value reading ends at end of file; created files stay readable. |
| `hackystl_test_Filesystem_FSRenameTest` | [Filesystem/FSRenameTest.cpp](Filesystem/FSRenameTest.cpp) | Renaming files and directories, listing after a rename through the same path object, replacing a file, and `create_directory` on existing and missing parents, in a temporary directory. |
| `hackystl_test_List_Single_SListEdgeTest` | [List/Single/SListEdgeTest.cpp](List/Single/SListEdgeTest.cpp) | Erasing the tail of a forward list followed by appends, and a seeded random sequence of operations compared with `std::forward_list`. |
| `hackystl_test_Time_DateOrderTest` | [Time/DateOrderTest.cpp](Time/DateOrderTest.cpp) | Calendar ordering across months and years, December and the last day of a month, equality between literal formats, and the weekday printed by `to_text()`. |
| `hackystl_test_Time_ClockUnitsTest` | [Time/ClockUnitsTest.cpp](Time/ClockUnitsTest.cpp) | Whole milliseconds and microseconds convert exactly, with no truncation from floating-point error. |
| `hackystl_test_Random_RandRangeEdgeTest` | [Random/RandRangeEdgeTest.cpp](Random/RandRangeEdgeTest.cpp) | Ranged draws stay inside `[from, to)` for negative integer ranges and float ranges, and empty or reversed ranges are rejected. |
| `hackystl_test_Algorithm_SortPredicateEdgeTest` | [Algorithm/SortPredicateEdgeTest.cpp](Algorithm/SortPredicateEdgeTest.cpp) | Predicate sorts order records that have no `operator>`. |
| `hackystl_test_Tuple_TupleEdgeTest` | [Tuple/TupleEdgeTest.cpp](Tuple/TupleEdgeTest.cpp) | `tie` with several references writes through to the variables, `make_tuple` from a const lvalue, and pair equality. |
| `hackystl_test_Variant_VarLvalueTest` | [Variant/VarLvalueTest.cpp](Variant/VarLvalueTest.cpp) | A variant constructed from an lvalue stores a copy of it. |
| `hackystl_test_SharedPtr_SharedFactoryEdgeTest` | [SharedPtr/SharedFactoryEdgeTest.cpp](SharedPtr/SharedFactoryEdgeTest.cpp) | The shared factories accept lvalue constructor arguments, and an allocator still selects the allocator overload. |
| `hackystl_test_Gorbitsa_GorMemoryEdgeTest` | [Gorbitsa/GorMemoryEdgeTest.cpp](Gorbitsa/GorMemoryEdgeTest.cpp) | The memory-writing instructions `O`, `o` and `r` at compile time. |
| `hackystl_test_Bitset_BitsetProxyEdgeTest` | [Bitset/BitsetProxyEdgeTest.cpp](Bitset/BitsetProxyEdgeTest.cpp) | Assigning one bit proxy to another writes the bit value. |
| `hackystl_test_ArgParse_ArgParseEdgeTest` | [ArgParse/ArgParseEdgeTest.cpp](ArgParse/ArgParseEdgeTest.cpp) | A missing trailing value is not read past `argc`, unknown options and unregistered help flags return, and a registered help flag runs its callback. |
| `hackystl_test_Vector_VecReverseEdgeTest` | [Vector/VecReverseEdgeTest.cpp](Vector/VecReverseEdgeTest.cpp) | Reverse traversal of empty and non-empty vectors, reverse views, string views in constant evaluation, and const access to fixed storage. |
| `hackystl_test_Ranges_RangeReverseEdgeTest` | [Ranges/RangeReverseEdgeTest.cpp](Ranges/RangeReverseEdgeTest.cpp) | Reverse views over string views, strings, vectors and empty sources. |
| `hackystl_test_Deque_DequeReverseEdgeTest` | [Deque/DequeReverseEdgeTest.cpp](Deque/DequeReverseEdgeTest.cpp) | Reverse traversal of empty, wrapped and const deques: it starts at the last element, ends after the first, and const access gives const elements. |
| `hackystl_test_Io_IoPrintGrowEdgeTest` | [Io/IoPrintGrowEdgeTest.cpp](Io/IoPrintGrowEdgeTest.cpp) | Output longer than the buffer it is written into: a dynamic stream with no storage yet, appends past its capacity, and an `io` file write longer than its 1024-character buffer. |
| `hackystl_test_Io_PrintBoundsEdgeTest` | [Io/PrintBoundsEdgeTest.cpp](Io/PrintBoundsEdgeTest.cpp) | `print_to` into a block of exactly the size it reports, for literals, numbers, floats, strings, bitsets, big integers, and styled and prefixed values. Each block is a heap allocation, so AddressSanitizer reports a write past its end. |
| `hackystl_test_Io_PrintTagEdgeTest` | [Io/PrintTagEdgeTest.cpp](Io/PrintTagEdgeTest.cpp) | Format tags a value cannot print, such as `{hex}` on a string, are rejected at compile time, for integers, floats, text, bitsets and big integers. Supported tags still print the expected text, including the eight-digit hex of a bitset. |
