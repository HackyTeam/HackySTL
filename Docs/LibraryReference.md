# HackySTL Library Reference {#library_reference}

This page is the module-level index for every header under `lib/`. Headers are
organized by subsystem and are intentionally small: the header owns the API,
while this page records its role, dependencies, and intended use. Public
headers are suitable for direct inclusion; files whose names begin with `_`
are implementation details normally included by another public header.

HackySTL targets C++23, uses the `hsd` namespace, and reports failures through
`result`, `option`, or `option_err` rather than C++ exceptions. Allocator-aware
owners preserve the allocator together with their storage. See
[Allocators](Allocators.md) for the complete allocator contract.

## Base

- [Base/Algorithm.hpp](../../lib/Base/Algorithm.hpp) - Iterator algorithms and small generic operations shared by containers and algorithms.
- [Base/Allocator.hpp](../../lib/Base/Allocator.hpp) - Raw allocators, buffered storage, allocation errors, and the typed `allocator_traits` customization point.
- [Base/Concepts.hpp](../../lib/Base/Concepts.hpp) - Project concepts describing constructibility, invocability, numeric types, iterators, and container capabilities.
- [Base/CString.hpp](../../lib/Base/CString.hpp) - Character-type-independent string operations, comparisons, parsing, copying, and whitespace helpers.
- [Base/_Define.hpp](../../lib/Base/_Define.hpp) - Compiler and platform feature macros used internally by the library.
- [Base/_Int128.hpp](../../lib/Base/_Int128.hpp) - Portable 128-bit integer used as `u128` and `i128` on MSVC, and on other compilers when `HSD_LIBRARY_INT128` is defined.
- [Base/Hash.hpp](../../lib/Base/Hash.hpp) - Hash primitives and hash specializations used by associative containers and string-like keys.
- [Base/IntegerSequence.hpp](../../lib/Base/IntegerSequence.hpp) - Compile-time integer, index, ranged, and inverse sequence generators.
- [Base/Limits.hpp](../../lib/Base/Limits.hpp) - Numeric limit constants used by project numeric implementations.
- [Base/Math.hpp](../../lib/Base/Math.hpp) - `constexpr` mathematical functions and numeric helpers.
- [Base/ReverseIterator.hpp](../../lib/Base/ReverseIterator.hpp) - Reverse iterators over contiguous storage and bidirectional containers that never form a pointer before the first element.
- [Base/Random.hpp](../../lib/Base/Random.hpp) - Deterministic 32-bit and 64-bit Mersenne Twister generators with bounded numeric results.
- [Base/Result.hpp](../../lib/Base/Result.hpp) - Move-oriented `result`, `option`, and `option_err` types plus project error and panic behavior.
- [Base/SourceLocation.hpp](../../lib/Base/SourceLocation.hpp) - Source-location abstraction used by diagnostics and error reporting.
- [Base/Switch.hpp](../../lib/Base/Switch.hpp) - Compile-time-friendly equality, ordering, and default dispatch cases.
- [Base/Types.hpp](../../lib/Base/Types.hpp) - Fundamental signed, unsigned, floating, character, size, and pointer aliases.
- [Base/TypeTraits.hpp](../../lib/Base/TypeTraits.hpp) - Project type traits, aliases, and trait-based concepts built on standard type traits.
- [Base/Unicode.hpp](../../lib/Base/Unicode.hpp) - Unicode length measurement and conversion between UTF-8, UTF-16, UTF-32, and wide character sequences.
- [Base/Utility.hpp](../../lib/Base/Utility.hpp) - Move/forward utilities, object lifetime helpers, bit operations, aligned raw allocation, and general algorithms.

## Containers

- [Containers/Bitset.hpp](../../lib/Containers/Bitset.hpp) - Fixed-width bit storage with whole-set, individual-bit, shift, count, and test operations.
- [Containers/Deque.hpp](../../lib/Containers/Deque.hpp) - Allocator-aware circular contiguous deque with front/back insertion, removal, resize, and random indexing.
- [Containers/ForwardList.hpp](../../lib/Containers/ForwardList.hpp) - Allocator-aware singly linked list with front/back insertion and iterator erasure.
- [Containers/HeapArray.hpp](../../lib/Containers/HeapArray.hpp) - Fixed-size owning array whose elements live in allocator-provided storage.
- [Containers/Integer.hpp](../../lib/Containers/Integer.hpp) - Fixed-width arbitrary-size integer operations, division, comparison, and formatting support.
- [Containers/List.hpp](../../lib/Containers/List.hpp) - Allocator-aware doubly linked list with bidirectional traversal and node erasure.
- [Containers/Range.hpp](../../lib/Containers/Range.hpp) - Lazy views and adaptors for drop, take, reverse, filtering, and transformation pipelines.
- [Containers/Span.hpp](../../lib/Containers/Span.hpp) - Non-owning contiguous views and enumeration support over existing storage.
- [Containers/StackArray.hpp](../../lib/Containers/StackArray.hpp) - Fixed-size inline array with checked access, fill, and standard-style iteration.
- [Containers/String.hpp](../../lib/Containers/String.hpp) - Allocator-aware null-terminated basic string with mutation, search, substring, conversion, and formatting operations.
- [Containers/StringView.hpp](../../lib/Containers/StringView.hpp) - Non-owning character sequence view with search, slicing, comparison, and literal support.
- [Containers/UnorderedMap.hpp](../../lib/Containers/UnorderedMap.hpp) - Allocator-aware hash map backed by value and bucket storage, with rehashing and duplicate-key handling.
- [Containers/VarTuple.hpp](../../lib/Containers/VarTuple.hpp) - Supporting tuple-like type machinery used by formatting and heterogeneous operations.
- [Containers/Vector.hpp](../../lib/Containers/Vector.hpp) - Allocator-aware dynamic vector and fixed-capacity `static_vector` with lifetime-safe growth and erasure.

## Extra

- [Extra/CompressedPair.hpp](../../lib/Extra/CompressedPair.hpp) - Pair storage that uses empty-base optimization for empty element types.
- [Extra/Constexpr/Gorbitsa.hpp](../../lib/Extra/Constexpr/Gorbitsa.hpp) - Compile-time parser and renderer for the Gorbitsa mini-language.
- [Extra/DefaultArg.hpp](../../lib/Extra/DefaultArg.hpp) - Callable wrapper supporting selected default arguments through `default_v`.
- [Extra/Json.hpp](../../lib/Extra/Json.hpp) - JSON lexer, parser, typed value tree, object/array access, and numeric/string conversion.
- [Extra/HTTP/HttpBase.hpp](../../lib/Extra/HTTP/HttpBase.hpp) - HTTP protocol, request-method, and response-status enums plus protocol validation helpers.
- [Extra/HTTP/HttpHeader.hpp](../../lib/Extra/HTTP/HttpHeader.hpp) - Case-insensitive HTTP field storage and CRLF header parsing.
- [Extra/HTTP/HttpRequest.hpp](../../lib/Extra/HTTP/HttpRequest.hpp) - HTTP request-line and header parser with method, path, version, and field access.
- [Extra/HTTP/HttpResponse.hpp](../../lib/Extra/HTTP/HttpResponse.hpp) - HTTP response status-line and header parser with status/message access.
- [Extra/HTTP/HttpSocket.hpp](../../lib/Extra/HTTP/HttpSocket.hpp) - Stateful HTTP socket request/response buffering built on the network socket layer.

## Filesystem

- [Filesystem/FStatus.hpp](../../lib/Filesystem/FStatus.hpp) - File and directory status, type predicates, size, and permission information.
- [Filesystem/Path.hpp](../../lib/Filesystem/Path.hpp) - Relative/absolute path representation, joining, listing, parents, copying, renaming, and directory creation.
- [Filesystem/_FSDetail.hpp](../../lib/Filesystem/_FSDetail.hpp) - Internal filesystem location and platform implementation details.

## Network

- [Network/Network.hpp](../../lib/Network/Network.hpp) - Public TCP and UDP client/server wrappers with polling, send, receive, and peer management.
- [Network/_NetworkDetail.hpp](../../lib/Network/_NetworkDetail.hpp) - Internal protocol, native handle, address, and platform definitions.
- [Network/_NetworkFuncs.hpp](../../lib/Network/_NetworkFuncs.hpp) - Internal socket creation, address resolution, error, and address formatting helpers.
- [Network/_SocketDetail.hpp](../../lib/Network/_SocketDetail.hpp) - Internal TCP/UDP socket ownership, polling, accept, send, receive, and child-socket implementations.

## Serialization and I/O

- [Serialize/ArgParse.hpp](../../lib/Serialize/ArgParse.hpp) - Command-line option registration, argument grouping, help information, and typed parser streams.
- [Serialize/FormatGenerator.hpp](../../lib/Serialize/FormatGenerator.hpp) - Compile-time format literal representation and formatting tag model.
- [Serialize/Io.hpp](../../lib/Serialize/Io.hpp) - File, stdin/stdout, buffered reads, typed parsing, formatted output, and stream helpers.
- [Serialize/LiteralParser.hpp](../../lib/Serialize/LiteralParser.hpp) - Compile-time parsing of format literals into format segments.
- [Serialize/Logging.hpp](../../lib/Serialize/Logging.hpp) - Per-thread call-site stack traces and diagnostic logging helpers.
- [Serialize/Tracing.hpp](../../lib/Serialize/Tracing.hpp) - Scope timing with per-thread nesting, serialized hooks, buffered text output, and the opt-in `HSD_TRACE_SCOPE` macro.
- [Serialize/SStream.hpp](../../lib/Serialize/SStream.hpp) - Dynamic and static string streams for formatted output and typed input parsing.
- [Serialize/StringLiteral.hpp](../../lib/Serialize/StringLiteral.hpp) - Compile-time string literal wrapper used as a non-type template parameter.
- [Serialize/StringParser.hpp](../../lib/Serialize/StringParser.hpp) - Compile-time format-driven parsing into numeric and string parser values.
- [Serialize/ToString.hpp](../../lib/Serialize/ToString.hpp) - Integer, floating-point, hexadecimal, and character conversion helpers.
- [Serialize/_FormatGeneratorDetail.hpp](../../lib/Serialize/_FormatGeneratorDetail.hpp) - Internal style and escape-sequence generation.
- [Serialize/_IoDetail.hpp](../../lib/Serialize/_IoDetail.hpp) - Internal platform-specific file descriptor and handle implementation.
- [Serialize/_PrinterImpl.hpp](../../lib/Serialize/_PrinterImpl.hpp) - Internal format dispatch and value printing implementation.
- [Serialize/_SStreamDetail.hpp](../../lib/Serialize/_SStreamDetail.hpp) - Internal stream tokenization, separator, and parsing helpers.
- [Serialize/_ToStringBase.hpp](../../lib/Serialize/_ToStringBase.hpp) - Internal fixed-buffer string conversion storage.
- [Serialize/_ToStringFloating.hpp](../../lib/Serialize/_ToStringFloating.hpp) - Internal floating-point conversion algorithms and buffers.

## Threading and time

- [Threading/Atomic.hpp](../../lib/Threading/Atomic.hpp) - Atomic values, flags, memory orders, compare/exchange, and arithmetic operations.
- [Threading/ConditionVariable.hpp](../../lib/Threading/ConditionVariable.hpp) - Condition-variable waiting, notification, and timed waiting primitives.
- [Threading/Future.hpp](../../lib/Threading/Future.hpp) - Promises, futures, packaged tasks, result propagation, and asynchronous waiting.
- [Threading/Lock.hpp](../../lib/Threading/Lock.hpp) - Mutex, futex, spin, unique-lock, and lock ownership abstractions.
- [Threading/MonotonicClock.hpp](../../lib/Threading/MonotonicClock.hpp) - Monotonic nanosecond clock for measuring elapsed time.
- [Threading/Thread.hpp](../../lib/Threading/Thread.hpp) - Thread creation, argument decay, joining, detaching, and thread identity.
- [Threading/Time.hpp](../../lib/Threading/Time.hpp) - Date/time values, literals, wall-clock and process clocks, durations, and time formatting.

## Wrappers

- [Wrappers/Any.hpp](../../lib/Wrappers/Any.hpp) - Copyable type-erased value storage with checked casts and in-place construction.
- [Wrappers/Functional.hpp](../../lib/Wrappers/Functional.hpp) - Type-erased callable storage with small-object optimization and result-aware invocation.
- [Wrappers/Pair.hpp](../../lib/Wrappers/Pair.hpp) - Project pair type, deduction guides, and tuple-like element access.
- [Wrappers/Reference.hpp](../../lib/Wrappers/Reference.hpp) - Non-owning references used in result values, containers, and type-erased APIs.
- [Wrappers/SharedPtr.hpp](../../lib/Wrappers/SharedPtr.hpp) - Atomic and non-atomic reference-counted owning pointers with allocator-aware storage.
- [Wrappers/Tuple.hpp](../../lib/Wrappers/Tuple.hpp) - Compile-time heterogeneous tuple storage, tuple construction, concatenation, and element access.
- [Wrappers/UniquePtr.hpp](../../lib/Wrappers/UniquePtr.hpp) - Move-only owning pointers for single objects and arrays with allocator-aware destruction.
- [Wrappers/Variant.hpp](../../lib/Wrappers/Variant.hpp) - Tagged heterogeneous storage with type/index access, visitation, and value-based comparison.

## Documentation expectations

When adding a header, add it to this page with its subsystem, public/detail
status, responsibility, and the most important ownership or failure contract.
Keep detailed parameter documentation next to the declaration it describes;
use this page for module-level architecture and navigation.
