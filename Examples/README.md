# Examples

Each program here shows one part of HackySTL and prints what it does. The
folders mirror [Tests/](../Tests/): one folder per feature, one program per
`.cpp`, and each feature has three examples at increasing complexity.

- **Basic** - one idea, a few calls.
- **Intermediate** - combines features or handles realistic input.
- **Advanced** - larger programs, compile-time use, threads, or several
  features at once.

Examples print their results and do not use `assert`. CTest checks the printed
output against a `.cases` file next to each program, following the Moodle VPL
`vpl_evaluate.cases` syntax.

## Build and run

From the repository root:

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target hackystl_example_Algorithm_SortBasics
./build/examples/hackystl_example_Algorithm_SortBasics
```

Run the checks with CTest:

```sh
ctest --test-dir build -L examples --output-on-failure
ctest --test-dir build -R '^hackystl_example_Algorithm_SortBasics$' --output-on-failure
```

Building needs only the C++ toolchain. Running the checks needs Python 3.8 or
newer, because `cmake/RunCases.py` is a standard-library Python script. Without
Python the examples still build, but their output is not checked.

Each example runs in `build/example_run/<target>`, so examples that write files
never touch the source tree. The runner clears that directory before each run.
The examples are built with the same warnings and sanitizers as the tests.
`HACKYSTL_BUILD_EXAMPLES=OFF` skips them entirely.

## Writing a `.cases` file

A `.cases` file sits beside its program and has the same base name. It holds one
or more cases. Statement names are case-insensitive. Each statement is written
as `name = value`, and a value may continue on the following lines until the
next statement.

```
Case = Default order is ascending, and a predicate can reverse it
Input =
Output = "ascending: 1 2 5 7 9
descending: 9 7 5 2 1
words: apple fig pear"
```

| Statement | Meaning |
| --- | --- |
| `Case` | Title shown in the report. Each `Case` starts a new run. |
| `Input` | Required once per case. Fed to the program as stdin. May be empty. |
| `Output` | Required, and may repeat. The case passes if any `Output` matches the stdout. |
| `Fail message` | Printed on failure in place of the default input, expected and actual details. |
| `Program arguments` | Command-line arguments for this run, with shell-style quoting. |
| `Expected exit code` | Defaults to `0`. |

The shape of an `Output` value selects how it is compared:

| Value | Comparison |
| --- | --- |
| `"text"` | Exact. The whole stdout must equal the text. CRLF is normalized, and trailing newlines are ignored on both sides. |
| `/pattern/flags` | Regular expression search. Flags: `i`, `m`, `s`, `x`. |
| Only numbers, such as `1 2.5 3` | The numbers in stdout, in order, must equal the expected numbers. Values with a decimal point or exponent compare with a relative tolerance of `1e-6`. Integers compare exactly. Other text in stdout is ignored. |
| Anything else | Loose text. Letters and digits must match, ignoring case, spaces and punctuation. |

A line that starts in column 0 with `word =` must be a known statement, so a
value line that looks like one must be indented. Lines starting with `#`
between statements are comments.

## Manual examples

Programs that need a peer, such as the TCP and UDP servers and clients, have no
`.cases` file. Their header says `// Manual:` and why. CTest lists them as
disabled, so they are built but not run. Start the matching peer in another
terminal and run them by hand, as described in the header.

## Shared helpers

`Support/ExampleSupport.hpp` holds helpers that more than one example uses. Examples
include it with a relative path, such as `#include "../Support/ExampleSupport.hpp"`.
Its `with_forward_slashes` prints paths with `/` on every platform, because Windows
reports them with `\`.

## Checking the tree

`cmake/CheckExamples.py` runs as the CTest entry `hackystl_examples_lint`. It
verifies that:

- each example has a `// Level:` header in its first ten lines,
- each example has a `.cases` file that parses, or a `// Manual:` header,
- examples do not use `assert`, `<cassert>` or `std::print`,
- each `.cases` file has a matching `.cpp`,
- each example's CTest target is listed in this file.

## Inventory

### Algorithm

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Algorithm_SortBasics` | [Algorithm/SortBasics.cpp](Algorithm/SortBasics.cpp) | Basic | `quick_sort` with the default order, a predicate that reverses it, and strings. |
| `hackystl_example_Algorithm_RadixSortRecords` | [Algorithm/RadixSortRecords.cpp](Algorithm/RadixSortRecords.cpp) | Intermediate | `radix_sort` on records by an unsigned key, stability, and the default unsigned sort. |
| `hackystl_example_Algorithm_SortCompileTime` | [Algorithm/SortCompileTime.cpp](Algorithm/SortCompileTime.cpp) | Advanced | Constexpr sorts checked by `static_assert`, and sorters that cover only a sub-range. |

### Allocator

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Allocator_AllocatorBasics` | [Allocator/AllocatorBasics.cpp](Allocator/AllocatorBasics.cpp) | Basic | The `allocator_traits` life cycle of allocate, construct, destroy and deallocate. |
| `hackystl_example_Allocator_BufferedAllocator` | [Allocator/BufferedAllocator.cpp](Allocator/BufferedAllocator.cpp) | Intermediate | A vector living in a stack buffer, allocation failure, and block reuse. |
| `hackystl_example_Allocator_CountingAllocator` | [Allocator/CountingAllocator.cpp](Allocator/CountingAllocator.cpp) | Advanced | A custom allocator with an `allocator_traits` specialization under vector and unordered_map. |

### Result

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Result_ResultBasics` | [Result/ResultBasics.cpp](Result/ResultBasics.cpp) | Basic | `result`, `option` and `option_err`, plus the `unwrap_or` family of recovery calls. |
| `hackystl_example_Result_ResultParsing` | [Result/ResultParsing.cpp](Result/ResultParsing.cpp) | Intermediate | Chaining failures by hand across functions and recovering with `unwrap_or_else`. |
| `hackystl_example_Result_ResultErrorTypes` | [Result/ResultErrorTypes.cpp](Result/ResultErrorTypes.cpp) | Advanced | Errors with `pretty_error`, `result` of `reference`, and error propagation through `option_err`. |

### Switch

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Switch_SwitchBasics` | [Switch/SwitchBasics.cpp](Switch/SwitchBasics.cpp) | Basic | `switch_cases` with `equal_case` on numbers and strings, and the `default_case` fallback. |
| `hackystl_example_Switch_SwitchRanges` | [Switch/SwitchRanges.cpp](Switch/SwitchRanges.cpp) | Intermediate | `less_case` and `greater_case` for banding values, and why the order of cases matters. |
| `hackystl_example_Switch_SwitchCommandDispatch` | [Switch/SwitchCommandDispatch.cpp](Switch/SwitchCommandDispatch.cpp) | Advanced | `switch_cases` as a command interpreter for a small stack machine. |

### Random

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Random_RandSeeded` | [Random/RandSeeded.cpp](Random/RandSeeded.cpp) | Basic | `mt19937_32` with a fixed seed, ranged and floating point draws, and reproducibility. |
| `hackystl_example_Random_RandMonteCarlo` | [Random/RandMonteCarlo.cpp](Random/RandMonteCarlo.cpp) | Intermediate | Seeded floating point draws estimating pi, and a dice simulation with a histogram. |
| `hackystl_example_Random_RandShuffleSimulation` | [Random/RandShuffleSimulation.cpp](Random/RandShuffleSimulation.cpp) | Advanced | A Fisher-Yates shuffle on a seeded engine, and statistics gathered over many shuffles. |

### Math

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Math_MathBasics` | [Math/MathBasics.cpp](Math/MathBasics.cpp) | Basic | Rounding, powers, roots, trigonometry and the pi constant from `hsd::math`. |
| `hackystl_example_Math_MathSeries` | [Math/MathSeries.cpp](Math/MathSeries.cpp) | Intermediate | Taylor and Leibniz series compared against `hsd::math`, plus a constexpr factorial. |
| `hackystl_example_Math_MathNumericIntegration` | [Math/MathNumericIntegration.cpp](Math/MathNumericIntegration.cpp) | Advanced | Trapezoid and Simpson rules, Newton's method, and the same maths evaluated at compile time. |

### Limits

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Limits_LimitsTable` | [Limits/LimitsTable.cpp](Limits/LimitsTable.cpp) | Basic | `min`, `max`, `is_signed` and `digits` from `hsd::limits` for the integer and floating point types. |
| `hackystl_example_Limits_LimitsOverflowChecks` | [Limits/LimitsOverflowChecks.cpp](Limits/LimitsOverflowChecks.cpp) | Intermediate | Detecting integer overflow before it happens with `limits`, plus compile-time checks. |
| `hackystl_example_Limits_LimitsSaturatingArithmetic` | [Limits/LimitsSaturatingArithmetic.cpp](Limits/LimitsSaturatingArithmetic.cpp) | Advanced | Generic saturating add, subtract and multiply built on `limits`, used for pixels and health bars. |

### Unicode

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Unicode_UniConvert` | [Unicode/UniConvert.cpp](Unicode/UniConvert.cpp) | Basic | `convert_to` from UTF-8 into UTF-32 and UTF-16, printing the code points. |
| `hackystl_example_Unicode_UniLengths` | [Unicode/UniLengths.cpp](Unicode/UniLengths.cpp) | Intermediate | Counting bytes, code points and UTF-16 units of UTF-8 strings. |
| `hackystl_example_Unicode_UniTranscodePipeline` | [Unicode/UniTranscodePipeline.cpp](Unicode/UniTranscodePipeline.cpp) | Advanced | UTF-8 to UTF-16 to UTF-32 and back, error results for small buffers, and constexpr conversion. |

### IntegerSequence

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_IntegerSequence_SeqBasics` | [IntegerSequence/SeqBasics.cpp](IntegerSequence/SeqBasics.cpp) | Basic | `integer_sequence`, `make_index_sequence`, and reading the values back with a fold. |
| `hackystl_example_IntegerSequence_SeqPackExpansion` | [IntegerSequence/SeqPackExpansion.cpp](IntegerSequence/SeqPackExpansion.cpp) | Intermediate | Turning an array into a pack of arguments, and ranged and inverse sequences. |
| `hackystl_example_IntegerSequence_SeqCompileTimeTable` | [IntegerSequence/SeqCompileTimeTable.cpp](IntegerSequence/SeqCompileTimeTable.cpp) | Advanced | Constexpr lookup tables built from index sequences, checked with `static_assert`. |

### Utils

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Utils_CompressedPairBasics` | [Utils/CompressedPairBasics.cpp](Utils/CompressedPairBasics.cpp) | Basic | `compressed_pair` accessors, indexed `get`, and the space saved for an empty member. |
| `hackystl_example_Utils_ConceptsDemo` | [Utils/ConceptsDemo.cpp](Utils/ConceptsDemo.cpp) | Intermediate | The ready-made concepts in `Concepts.hpp` used to constrain templates and pick overloads. |
| `hackystl_example_Utils_BitScanAndRawStorage` | [Utils/BitScanAndRawStorage.cpp](Utils/BitScanAndRawStorage.cpp) | Advanced | `countl_zero` and `countr_zero` in constant expressions, and aligned storage built by hand. |

### DefaultArg

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_DefaultArg_DefaultArgBasics` | [DefaultArg/DefaultArgBasics.cpp](DefaultArg/DefaultArgBasics.cpp) | Basic | `default_call_t` storing fallback values, and `default_v` selecting them at the call. |
| `hackystl_example_DefaultArg_DefaultArgPartialOverride` | [DefaultArg/DefaultArgPartialOverride.cpp](DefaultArg/DefaultArgPartialOverride.cpp) | Intermediate | `default_call_t` over a lambda, a `make_defaultcall` signature, and defaults in any position. |
| `hackystl_example_DefaultArg_DefaultArgCallbacks` | [DefaultArg/DefaultArgCallbacks.cpp](DefaultArg/DefaultArgCallbacks.cpp) | Advanced | `default_call_t` wrapped in `hsd::function`, a pipeline of stages, and constexpr use. |

### Vector

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Vector_VecBasics` | [Vector/VecBasics.cpp](Vector/VecBasics.cpp) | Basic | `push_back`, `emplace_back`, checked `at()`, `erase`, `resize` and `reserve` on `hsd::vector`. |
| `hackystl_example_Vector_VecMatrix` | [Vector/VecMatrix.cpp](Vector/VecMatrix.cpp) | Intermediate | Nested `hsd::vector` as a matrix: transpose, multiplication and row sums, with checked access. |
| `hackystl_example_Vector_StaticVecRingBuffer` | [Vector/StaticVecRingBuffer.cpp](Vector/StaticVecRingBuffer.cpp) | Advanced | `static_vector` with fixed capacity, a ring buffer built on it, and constexpr use checked by `static_assert`. |

### String

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_String_StringBasics` | [String/StringBasics.cpp](String/StringBasics.cpp) | Basic | Building `hsd::string`, length versus size, concatenation, searching and `sub_string`. |
| `hackystl_example_String_StringEditing` | [String/StringEditing.cpp](String/StringEditing.cpp) | Intermediate | Trim, collapse spaces, replace and reverse written on `hsd::string`, applied to lines read from stdin. |
| `hackystl_example_String_StringTemplateEngine` | [String/StringTemplateEngine.cpp](String/StringTemplateEngine.cpp) | Advanced | `hsd::string` with `unordered_map` to fill `{{name}}` placeholders, with defaults and escapes, from `key: value` lines on stdin. |
| `hackystl_example_String_ViewBasics` | [String/ViewBasics.cpp](String/ViewBasics.cpp) | Basic | `hsd::string_view` as a non-owning window: `find`, `rfind`, prefix and suffix tests, `sub_string`, and constexpr use. |
| `hackystl_example_String_ViewTokenizer` | [String/ViewTokenizer.cpp](String/ViewTokenizer.cpp) | Intermediate | Splitting text into tokens with `string_view` and `find_first_of`, with no copies, over lines read from stdin. |
| `hackystl_example_String_ViewCsvParser` | [String/ViewCsvParser.cpp](String/ViewCsvParser.cpp) | Advanced | A quote-aware CSV reader whose cells are `string_view` slices of the input, with column lookup by header and checked totals. |

### UnorderedMap

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_UnorderedMap_UMapBasics` | [UnorderedMap/UMapBasics.cpp](UnorderedMap/UMapBasics.cpp) | Basic | `emplace`, `operator[]`, checked `at()`, size and erase on `unordered_map`. |
| `hackystl_example_UnorderedMap_UMapWordFrequency` | [UnorderedMap/UMapWordFrequency.cpp](UnorderedMap/UMapWordFrequency.cpp) | Intermediate | Counting words from stdin with an `unordered_map` and ranking them. |
| `hackystl_example_UnorderedMap_UMapCustomKeyMemo` | [UnorderedMap/UMapCustomKeyMemo.cpp](UnorderedMap/UMapCustomKeyMemo.cpp) | Advanced | A struct key with its own hasher, memoizing a recursive grid count, copying and reserving. |

### List

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_List_Double_DListBasics` | [List/Double/DListBasics.cpp](List/Double/DListBasics.cpp) | Basic | Push and pop at both ends, `front`/`back`, iteration both ways, and erase. |
| `hackystl_example_List_Double_DListEditorBuffer` | [List/Double/DListEditorBuffer.cpp](List/Double/DListEditorBuffer.cpp) | Intermediate | Two doubly linked lists around a cursor, driven by commands read from stdin. |
| `hackystl_example_List_Double_DListLruCache` | [List/Double/DListLruCache.cpp](List/Double/DListLruCache.cpp) | Advanced | A doubly linked list plus an `unordered_map` as a least-recently-used cache. |
| `hackystl_example_List_Single_SListBasics` | [List/Single/SListBasics.cpp](List/Single/SListBasics.cpp) | Basic | `forward_list` push, pop, `front`/`back`, iteration, erase and size. |
| `hackystl_example_List_Single_SListStack` | [List/Single/SListStack.cpp](List/Single/SListStack.cpp) | Intermediate | `push_front`/`pop_front` as a stack, in an RPN calculator reading stdin. |
| `hackystl_example_List_Single_SListMergeSorted` | [List/Single/SListMergeSorted.cpp](List/Single/SListMergeSorted.cpp) | Advanced | Merging two sorted `forward_list`s, a merge sort built on it, and de-duplication. |

### Deque

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Deque_DequeBasics` | [Deque/DequeBasics.cpp](Deque/DequeBasics.cpp) | Basic | Pushing and popping at both ends, indexing, checked `at()`, erase, and reverse iteration. |
| `hackystl_example_Deque_DequeSlidingWindow` | [Deque/DequeSlidingWindow.cpp](Deque/DequeSlidingWindow.cpp) | Intermediate | A monotonic deque that keeps window maxima, using `front`, `back` and both pops. |
| `hackystl_example_Deque_DequeBfsGrid` | [Deque/DequeBfsGrid.cpp](Deque/DequeBfsGrid.cpp) | Advanced | A deque as a BFS queue, a 0-1 BFS using `push_front`, and path rebuilding. |

### Array

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Array_Stack_StackArrBasics` | [Array/Stack/StackArrBasics.cpp](Array/Stack/StackArrBasics.cpp) | Basic | `stack_array` construction, checked `at()`, fill, swap and iteration. |
| `hackystl_example_Array_Stack_StackArrMatrix` | [Array/Stack/StackArrMatrix.cpp](Array/Stack/StackArrMatrix.cpp) | Intermediate | Nested `stack_array` as a fixed-size matrix, transpose and multiplication. |
| `hackystl_example_Array_Stack_StackArrConstexpr` | [Array/Stack/StackArrConstexpr.cpp](Array/Stack/StackArrConstexpr.cpp) | Advanced | Lookup tables built in constexpr functions and checked with `static_assert`. |
| `hackystl_example_Array_Heap_HeapArrBasics` | [Array/Heap/HeapArrBasics.cpp](Array/Heap/HeapArrBasics.cpp) | Basic | `heap_array` construction from a C array, checked `at()`, and iteration. |
| `hackystl_example_Array_Heap_HeapArrCopyMove` | [Array/Heap/HeapArrCopyMove.cpp](Array/Heap/HeapArrCopyMove.cpp) | Intermediate | Deep copy, element-wise copy assignment, and move construction and assignment. |
| `hackystl_example_Array_Heap_HeapArrImageBuffer` | [Array/Heap/HeapArrImageBuffer.cpp](Array/Heap/HeapArrImageBuffer.cpp) | Advanced | A `heap_array` as a pixel buffer with a custom element type, shapes, and copy-on-snapshot. |

### Span

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Span_SpanBasics` | [Span/SpanBasics.cpp](Span/SpanBasics.cpp) | Basic | Viewing a container with `span`, `size`, `take`, `drop`, `enumerate`, and editing through a view. |
| `hackystl_example_Span_SpanWindows` | [Span/SpanWindows.cpp](Span/SpanWindows.cpp) | Intermediate | Sliding windows and fixed-size chunks built from `drop` and `take`. |
| `hackystl_example_Span_SpanAlgorithms` | [Span/SpanAlgorithms.cpp](Span/SpanAlgorithms.cpp) | Advanced | Generic algorithms over spans of different containers, `enumerate`, and constexpr spans. |

### Bitset

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Bitset_BitsetBasics` | [Bitset/BitsetBasics.cpp](Bitset/BitsetBasics.cpp) | Basic | Building a bitset, `set`/`reset`/`flip`/`test`, counting, bit operators and shifts. |
| `hackystl_example_Bitset_BitsetSieve` | [Bitset/BitsetSieve.cpp](Bitset/BitsetSieve.cpp) | Intermediate | A large bitset as a compact flag table, driven by a limit read from stdin. |
| `hackystl_example_Bitset_BitsetGameOfLife` | [Bitset/BitsetGameOfLife.cpp](Bitset/BitsetGameOfLife.cpp) | Advanced | One bitset per row, bit-parallel neighbour counting, and a per-cell reference check. |

### Integer

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Integer_IntegerBasics` | [Integer/IntegerBasics.cpp](Integer/IntegerBasics.cpp) | Basic | `integer<N>` arithmetic that wraps modulo 2^N, division with remainder, and widening. |
| `hackystl_example_Integer_IntegerFactorial` | [Integer/IntegerFactorial.cpp](Integer/IntegerFactorial.cpp) | Intermediate | `integer<512>` products read from stdin, digit sums by division, and 64-bit wrap-around. |
| `hackystl_example_Integer_IntegerFibonacci` | [Integer/IntegerFibonacci.cpp](Integer/IntegerFibonacci.cpp) | Advanced | Iterative and fast-doubling Fibonacci in `integer<1024>`, a `static_assert` check, and gcd. |

### Ranges

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Ranges_RangeBasics` | [Ranges/RangeBasics.cpp](Ranges/RangeBasics.cpp) | Basic | `drop`, `take`, `drop_while`, `take_while`, `reverse` and span `enumerate`. |
| `hackystl_example_Ranges_RangePipelines` | [Ranges/RangePipelines.cpp](Ranges/RangePipelines.cpp) | Intermediate | Chaining `filter`, `transform`, `drop_while`, `take` and `reverse` over numbers read from stdin. |
| `hackystl_example_Ranges_RangeTextProcessing` | [Ranges/RangeTextProcessing.cpp](Ranges/RangeTextProcessing.cpp) | Advanced | Trimming, word splitting and a palindrome test built only from range views on stdin lines. |

### Any

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Any_AnyBasics` | [Any/AnyBasics.cpp](Any/AnyBasics.cpp) | Basic | Storing a value in `hsd::any`, checking its type, casting, copying and resetting. |
| `hackystl_example_Any_AnyHeterogeneousBag` | [Any/AnyHeterogeneousBag.cpp](Any/AnyHeterogeneousBag.cpp) | Intermediate | A vector of `any` holding mixed types, dispatching on the stored type, and user-defined payloads. |
| `hackystl_example_Any_AnyPropertyMap` | [Any/AnyPropertyMap.cpp](Any/AnyPropertyMap.cpp) | Advanced | A string-keyed map of `any` as a typed property bag, with defaults, type errors and overwriting. |

### Functional

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Functional_FuncBasics` | [Functional/FuncBasics.cpp](Functional/FuncBasics.cpp) | Basic | `hsd::function` holding a lambda, a function pointer and a stateful lambda, and the empty state. |
| `hackystl_example_Functional_FuncEventBus` | [Functional/FuncEventBus.cpp](Functional/FuncEventBus.cpp) | Intermediate | Storing `hsd::function` callbacks in a vector and a map, calling them, and removing a handler. |
| `hackystl_example_Functional_FuncPipelineBind` | [Functional/FuncPipelineBind.cpp](Functional/FuncPipelineBind.cpp) | Advanced | Factories returning `hsd::function`, partial application with captures, composition, and a stage pipeline. |

### Tuple

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Tuple_TupleBasics` | [Tuple/TupleBasics.cpp](Tuple/TupleBasics.cpp) | Basic | Building a tuple, reading elements by index, `make_tuple`, concatenation and `tie`. |
| `hackystl_example_Tuple_PairStructuredBindings` | [Tuple/PairStructuredBindings.cpp](Tuple/PairStructuredBindings.cpp) | Intermediate | `hsd::pair` with structured bindings, pairs in a vector, and why `tuple` needs `get<Index>()` instead. |
| `hackystl_example_Tuple_TupleCompileTimeApply` | [Tuple/TupleCompileTimeApply.cpp](Tuple/TupleCompileTimeApply.cpp) | Advanced | Constexpr tuples, `hsd::apply` with free callables and member functions, and `static_assert` checks. |

### Variant

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Variant_VariantBasics` | [Variant/VariantBasics.cpp](Variant/VariantBasics.cpp) | Basic | `hsd::variant` holding one of several types, with `index`, `holds`, `get` and `get_if`. |
| `hackystl_example_Variant_VariantVisitor` | [Variant/VariantVisitor.cpp](Variant/VariantVisitor.cpp) | Intermediate | `visit` with a visitor struct and with a generic lambda, over a vector of shape variants. |
| `hackystl_example_Variant_VariantExpressionEvaluator` | [Variant/VariantExpressionEvaluator.cpp](Variant/VariantExpressionEvaluator.cpp) | Advanced | Variants as token and result types, a recursive descent parser, and `visit` over stdin expressions. |

### UniquePtr

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_UniquePtr_UniqueBasics` | [UniquePtr/UniqueBasics.cpp](UniquePtr/UniqueBasics.cpp) | Basic | `make_unique`, move-only ownership, assigning null, and arrays owned by a `unique_ptr`. |
| `hackystl_example_UniquePtr_UniquePolymorphic` | [UniquePtr/UniquePolymorphic.cpp](UniquePtr/UniquePolymorphic.cpp) | Intermediate | A `unique_ptr` to a base class, a factory, a vector of owners, and handing ownership between containers. |
| `hackystl_example_UniquePtr_UniqueTreeOwnership` | [UniquePtr/UniqueTreeOwnership.cpp](UniquePtr/UniqueTreeOwnership.cpp) | Advanced | A binary search tree where each node owns its children, re-linking subtrees by moving owners. |

### SharedPtr

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_SharedPtr_SharedBasics` | [SharedPtr/SharedBasics.cpp](SharedPtr/SharedBasics.cpp) | Basic | `unsafe_shared_ptr` and `safe_shared_ptr` with copies, `get_count`, `is_unique` and reset by `nullptr`. |
| `hackystl_example_SharedPtr_SharedGraph` | [SharedPtr/SharedGraph.cpp](SharedPtr/SharedGraph.cpp) | Intermediate | Nodes shared by several parents through `unsafe_shared_ptr`, reference counts, and release order. |
| `hackystl_example_SharedPtr_SharedAcrossThreads` | [SharedPtr/SharedAcrossThreads.cpp](SharedPtr/SharedAcrossThreads.cpp) | Advanced | A `safe_shared_ptr` copied into worker threads, atomic reference counts under contention, and deterministic results. |

### Thread

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Thread_ThreadHello` | [Thread/ThreadHello.cpp](Thread/ThreadHello.cpp) | Basic | Starting a `hsd::thread`, passing arguments, joining, and the joinable state. |
| `hackystl_example_Thread_ThreadParallelSum` | [Thread/ThreadParallelSum.cpp](Thread/ThreadParallelSum.cpp) | Intermediate | A vector of threads, per-thread result slots, and joining in order. |
| `hackystl_example_Thread_ThreadPipeline` | [Thread/ThreadPipeline.cpp](Thread/ThreadPipeline.cpp) | Advanced | Three stages on separate threads connected by mutex and condition variable queues. |

### Lock

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Lock_LockCounter` | [Lock/LockCounter.cpp](Lock/LockCounter.cpp) | Basic | Protecting a shared counter with `hsd::mutex` and `hsd::unique_lock`. |
| `hackystl_example_Lock_LockCompareTypes` | [Lock/LockCompareTypes.cpp](Lock/LockCompareTypes.cpp) | Intermediate | `hsd::mutex`, `hsd::futex` and `hsd::spin` behind the same `unique_lock` code. |
| `hackystl_example_Lock_LockBankTransfer` | [Lock/LockBankTransfer.cpp](Lock/LockBankTransfer.cpp) | Advanced | One mutex per account, deferred locking, and lock ordering to avoid deadlock. |

### CondVar

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_CondVar_CondVarPingPong` | [CondVar/CondVarPingPong.cpp](CondVar/CondVarPingPong.cpp) | Basic | Two threads taking turns with a `condition_variable` and a predicate. |
| `hackystl_example_CondVar_CondVarProducerConsumer` | [CondVar/CondVarProducerConsumer.cpp](CondVar/CondVarProducerConsumer.cpp) | Intermediate | A bounded queue with two condition variables, several consumers, and a done flag. |
| `hackystl_example_CondVar_CondVarTimeout` | [CondVar/CondVarTimeout.cpp](CondVar/CondVarTimeout.cpp) | Advanced | `wait_for` and `wait_until` with and without a predicate, and `cv_status`. |

### Atomic

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Atomic_AtomicOps` | [Atomic/AtomicOps.cpp](Atomic/AtomicOps.cpp) | Basic | `load`, `store`, `exchange`, `compare_exchange_strong`, `fetch_*` and `atomic_flag`. |
| `hackystl_example_Atomic_AtomicCounters` | [Atomic/AtomicCounters.cpp](Atomic/AtomicCounters.cpp) | Intermediate | Lock-free counters, a ticket dispenser built on `fetch_add`, and `atomic_bool`. |
| `hackystl_example_Atomic_AtomicSpinLockCasMax` | [Atomic/AtomicSpinLockCasMax.cpp](Atomic/AtomicSpinLockCasMax.cpp) | Advanced | A spin lock built on `atomic_flag`, and a lock-free maximum with a compare-exchange loop. |

### Future

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Future_FuturePromise` | [Future/FuturePromise.cpp](Future/FuturePromise.cpp) | Basic | Handing a value from a worker thread to main with `promise` and `future`. |
| `hackystl_example_Future_FutureFanOut` | [Future/FutureFanOut.cpp](Future/FutureFanOut.cpp) | Intermediate | `packaged_task` on several threads, with futures collected in order. |
| `hackystl_example_Future_FutureErrors` | [Future/FutureErrors.cpp](Future/FutureErrors.cpp) | Advanced | Tasks returning `result`, error propagation, `option_err` tasks, and timed waits. |

### Time

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Time_TimeDateLiterals` | [Time/TimeDateLiterals.cpp](Time/TimeDateLiterals.cpp) | Basic | `_dmy` and `_mdy` literals, the time getters, equality, and compile-time checks. |
| `hackystl_example_Time_TimeDurations` | [Time/TimeDurations.cpp](Time/TimeDurations.cpp) | Intermediate | `precise_clock` literals, arithmetic with carry, comparison, and unit conversion. |
| `hackystl_example_Time_TimeStopwatch` | [Time/TimeStopwatch.cpp](Time/TimeStopwatch.cpp) | Advanced | Measuring with `monotonic_clock` and `precise_clock`, restart, and `sleep_for`. |

### Logger

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Logger_LogStackTrace` | [Logger/LogStackTrace.cpp](Logger/LogStackTrace.cpp) | Basic | `stack_trace::add` as an RAII call-site marker, and reading the top entry. |
| `hackystl_example_Logger_LogMiniLogger` | [Logger/LogMiniLogger.cpp](Logger/LogMiniLogger.cpp) | Intermediate | A small logger that tags messages with the call site taken from `stack_trace`. |
| `hackystl_example_Logger_LogErrorChain` | [Logger/LogErrorChain.cpp](Logger/LogErrorChain.cpp) | Advanced | `stack_trace_error` propagated through nested calls, with a `stack_trace` handle per frame. |

### Tracing

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Tracing_TraceHook` | [Tracing/TraceHook.cpp](Tracing/TraceHook.cpp) | Basic | `HSD_TRACE_SCOPE` with a custom `trace_sink` hook that prints names and depth only. |
| `hackystl_example_Tracing_TraceCallTree` | [Tracing/TraceCallTree.cpp](Tracing/TraceCallTree.cpp) | Intermediate | Tracing a recursive function and summarizing the events by depth in a hook. |
| `hackystl_example_Tracing_TraceThreads` | [Tracing/TraceThreads.cpp](Tracing/TraceThreads.cpp) | Advanced | Per-thread nesting depth, serialized hooks, and tracing from worker threads. |

### Io

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Io_IoFormatting` | [Io/IoFormatting.cpp](Io/IoFormatting.cpp) | Basic | `print` and `println` with `_fmt` literals, `{hex}`, `{prec=N}`, `{exp}`, chars and strings. |
| `hackystl_example_Io_IoStringStreams` | [Io/IoStringStreams.cpp](Io/IoStringStreams.cpp) | Basic | `sstream` `write_data`, `append_data` and `set_data`, a stream that grows from no `reserve()`, and a compile-time `static_sstream`. |
| `hackystl_example_Io_IoFileRoundTrip` | [Io/IoFileRoundTrip.cpp](Io/IoFileRoundTrip.cpp) | Intermediate | `io::load_file` for reading a fixture back, `flush`, `read_chunk`, `set_data`, and a print longer than the io buffer. |
| `hackystl_example_Io_IoStdinCalculator` | [Io/IoStdinCalculator.cpp](Io/IoStdinCalculator.cpp) | Intermediate | Reading stdin with `io::cin`, then parsing each command line with `sstream::set_data`. |
| `hackystl_example_Io_IoCustomTypes` | [Io/IoCustomTypes.cpp](Io/IoCustomTypes.cpp) | Advanced | The `{ovr}` tag with `pretty_fmt` and `pretty_args`, nested user types, and `sstream` output. |

### ToString

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_ToString_ToStringBasics` | [ToString/ToStringBasics.cpp](ToString/ToStringBasics.cpp) | Basic | `hsd::to_string` for owning strings and `to_pseudo_string` for fixed buffers. |
| `hackystl_example_ToString_ToStringFloats` | [ToString/ToStringFloats.cpp](ToString/ToStringFloats.cpp) | Intermediate | Fixed, scientific and hexadecimal float text, precision, rounding and special values. |
| `hackystl_example_ToString_ToStringConstexpr` | [ToString/ToStringConstexpr.cpp](ToString/ToStringConstexpr.cpp) | Advanced | `to_pseudo_string` in constant expressions, building a text table at compile time. |

### StringParser

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_StringParser_ParserBasics` | [StringParser/ParserBasics.cpp](StringParser/ParserBasics.cpp) | Basic | `parse_string` with `{}` placeholders, typed field conversion, and mismatches. |
| `hackystl_example_StringParser_ParserLogLines` | [StringParser/ParserLogLines.cpp](StringParser/ParserLogLines.cpp) | Intermediate | `parse_string` over a list of lines, per-level tallies, and skipping malformed lines. |
| `hackystl_example_StringParser_ParserKeyValueConfig` | [StringParser/ParserKeyValueConfig.cpp](StringParser/ParserKeyValueConfig.cpp) | Advanced | Reading config lines from stdin and combining `parse_string` patterns for sections and keys. |

### ArgParse

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_ArgParse_ArgParseBasics` | [ArgParse/ArgParseBasics.cpp](ArgParse/ArgParseBasics.cpp) | Basic | Registering options with `argument_parser` and parsing a prepared `argv`. |
| `hackystl_example_ArgParse_ArgParseCallbacks` | [ArgParse/ArgParseCallbacks.cpp](ArgParse/ArgParseCallbacks.cpp) | Intermediate | Flags, multi-token options, repeated options and error reporting from callbacks. |
| `hackystl_example_ArgParse_ArgParseCli` | [ArgParse/ArgParseCli.cpp](ArgParse/ArgParseCli.cpp) | Advanced | A small CLI built on `argument_parser`, with validation done before `parse()`. |

### Json

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Json_JsonBasics` | [Json/JsonBasics.cpp](Json/JsonBasics.cpp) | Basic | Lexing and parsing a JSON string, then reading typed fields with checked access. |
| `hackystl_example_Json_JsonWalker` | [Json/JsonWalker.cpp](Json/JsonWalker.cpp) | Intermediate | Lexing input in fragments, walking a nested tree recursively, and reporting syntax errors. |
| `hackystl_example_Json_JsonFileReport` | [Json/JsonFileReport.cpp](Json/JsonFileReport.cpp) | Advanced | `lex_file` on a fixture, safe lookups with `access()`, and a summary computed from the tree. |

### Network

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Network_HTTP_HttpParseRequest` | [Network/HTTP/HttpParseRequest.cpp](Network/HTTP/HttpParseRequest.cpp) | Basic | `http::request::parse`, reading method, path, version and header fields, and a rejected request. |
| `hackystl_example_Network_HTTP_HttpBuildResponse` | [Network/HTTP/HttpBuildResponse.cpp](Network/HTTP/HttpBuildResponse.cpp) | Intermediate | The response builder, writing the wire format by hand, and parsing it back. |
| `hackystl_example_Network_HTTP_HttpJsonExchange` | [Network/HTTP/HttpJsonExchange.cpp](Network/HTTP/HttpJsonExchange.cpp) | Advanced | Request and response as wire text, a routing and validating handler, and JSON bodies parsed in both directions. |
| `hackystl_example_Network_TCP_TcpEchoLoopback` | [Network/TCP/TcpEchoLoopback.cpp](Network/TCP/TcpEchoLoopback.cpp) | Basic | `tcp_server_v4` and `tcp_client_v4` in one process, an ephemeral port, and a server thread joined before printing. |
| `hackystl_example_Network_TCP_TcpMultiClientLoopback` | [Network/TCP/TcpMultiClientLoopback.cpp](Network/TCP/TcpMultiClientLoopback.cpp) | Advanced | One polling server thread serving three client threads, erasing finished sockets, and join-then-print reporting. |
| `hackystl_example_Network_TCP_TcpChatServer` | [Network/TCP/TcpChatServer.cpp](Network/TCP/TcpChatServer.cpp) | Intermediate | Manual: a polling `tcp_server_v4` that relays every line to the other clients. Needs `TcpChatClient`. |
| `hackystl_example_Network_TCP_TcpChatClient` | [Network/TCP/TcpChatClient.cpp](Network/TCP/TcpChatClient.cpp) | Intermediate | Manual: a `tcp_client_v4` that sends stdin lines while a thread prints relayed lines. Needs `TcpChatServer`. |
| `hackystl_example_Network_UDP_UdpLoopback` | [Network/UDP/UdpLoopback.cpp](Network/UDP/UdpLoopback.cpp) | Basic | `udp_server_v4` and `udp_client_v4` in one thread, an ephemeral port, and one reply per datagram. |
| `hackystl_example_Network_UDP_UdpRequestReply` | [Network/UDP/UdpRequestReply.cpp](Network/UDP/UdpRequestReply.cpp) | Advanced | A key-value server thread answering text commands from a UDP client, printed after join. |
| `hackystl_example_Network_UDP_UdpLogCollector` | [Network/UDP/UdpLogCollector.cpp](Network/UDP/UdpLogCollector.cpp) | Intermediate | Manual: a `udp_server_v4` that numbers datagrams, tallies levels, and stops on `/stop`. Needs `UdpLogSender`. |
| `hackystl_example_Network_UDP_UdpLogSender` | [Network/UDP/UdpLogSender.cpp](Network/UDP/UdpLogSender.cpp) | Intermediate | Manual: a `udp_client_v4` that sends each stdin line as one datagram. Needs `UdpLogCollector`. |

### Filesystem

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Filesystem_FsInspect` | [Filesystem/FsInspect.cpp](Filesystem/FsInspect.cpp) | Basic | `path::status()`, existence, type, size and permission queries, `extension()`, and a missing path. |
| `hackystl_example_Filesystem_FsScratchDirectory` | [Filesystem/FsScratchDirectory.cpp](Filesystem/FsScratchDirectory.cpp) | Intermediate | Creating a directory and files, sorted listing, copying a file, and reading the copy back. |
| `hackystl_example_Filesystem_FsTreeReport` | [Filesystem/FsTreeReport.cpp](Filesystem/FsTreeReport.cpp) | Advanced | Building a project tree, a recursive sorted walk, per-directory totals, and an extension tally. |

### Gorbitsa

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Gorbitsa_GorHello` | [Gorbitsa/GorHello.cpp](Gorbitsa/GorHello.cpp) | Basic | Running a Gorbitsa program at compile time with `run_parser` and reading its output array. |
| `hackystl_example_Gorbitsa_GorStairs` | [Gorbitsa/GorStairs.cpp](Gorbitsa/GorStairs.cpp) | Intermediate | Generating Gorbitsa source in a constexpr function and drawing a letter staircase with S, T and I. |
| `hackystl_example_Gorbitsa_GorCompileTimeProgram` | [Gorbitsa/GorCompileTimeProgram.cpp](Gorbitsa/GorCompileTimeProgram.cpp) | Advanced | Compiling a message to Gorbitsa in a constexpr function, running it at compile time, and branching with B. |

### Problems

| CTest target | Source | Level | Shows |
| --- | --- | --- | --- |
| `hackystl_example_Problems_WordFrequencyReport` | [Problems/WordFrequencyReport.cpp](Problems/WordFrequencyReport.cpp) | Intermediate | Tokenizing a text, counting in an `unordered_map` and ranking with `quick_sort`. |
| `hackystl_example_Problems_ExpenseTracker` | [Problems/ExpenseTracker.cpp](Problems/ExpenseTracker.cpp) | Intermediate | A stdin command loop with `result`-based amount parsing, budgets and undo. |
| `hackystl_example_Problems_TaskScheduler` | [Problems/TaskScheduler.cpp](Problems/TaskScheduler.cpp) | Advanced | Dependency resolution with priorities, cycle detection and a critical path, using `result` errors. |
| `hackystl_example_Problems_LogAnalyzer` | [Problems/LogAnalyzer.cpp](Problems/LogAnalyzer.cpp) | Intermediate | `parse_string` on log lines, per-service statistics and a filtered range of slow requests. |
| `hackystl_example_Problems_JsonInventory` | [Problems/JsonInventory.cpp](Problems/JsonInventory.cpp) | Intermediate | Parsing JSON with `JsonStream` and `JsonParser`, then reading it with checked access. |
| `hackystl_example_Problems_MiniHttpEcho` | [Problems/MiniHttpEcho.cpp](Problems/MiniHttpEcho.cpp) | Advanced | HTTP requests and responses carried over a loopback UDP socket pair inside one process. |
| `hackystl_example_Problems_ParallelWordCount` | [Problems/ParallelWordCount.cpp](Problems/ParallelWordCount.cpp) | Advanced | Threads counting chunks into private slots, an atomic total and a merge on the main thread. |
| `hackystl_example_Problems_CalculatorRepl` | [Problems/CalculatorRepl.cpp](Problems/CalculatorRepl.cpp) | Advanced | A recursive descent parser over stdin lines, with variables, functions and `result` errors. |
