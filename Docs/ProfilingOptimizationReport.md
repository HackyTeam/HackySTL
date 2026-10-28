# Profiling-driven optimization notes

## What the profile showed

The optimized C++23 profiling target exposed several costs in the custom
containers and wrappers:

- `deque` iterators performed a remainder operation for every dereference.
  End insertion also paid that cost when resolving the construction slot.
- `basic_string_view::find` scanned every possible start position and then
  called the library's generic compare loop for each first-character match.
- `unordered_map` grew its bucket vector by only 1.5x, causing frequent full
  bucket rebuilds during unreserved insertion. It also recalculated a key's
  hash after insertion.
- `any` used `dynamic_cast` for repeated type checks, and copying an empty
  `any` dereferenced a null holder.
- The insertion microbenchmarks read only size and two edge elements. An
  optimizing compiler could remove work on interior values, which made some
  reported numbers misleadingly low. A post-loop memory barrier now preserves
  the final container state during the measurement.

## Changes made

- Cache the physical element pointer in deque iterators, updating it with
  pointer increments/decrements and a wrap check. This removes integer modulo
  from the hot dereference path. Iterator equality now also compares the
  backing allocation, and `clear()` resets the cached pointers with the logical
  positions.
- Use bounded `std::char_traits<CharT>::find` and `compare` in string-view
  substring search. This preserves view bounds and lets implementations use
  optimized character-search primitives.
- Expose the concrete `std::type_info` through the erased `any` base and use
  exact type-info comparisons before a safe cast. Copying an empty `any` now
  leaves the destination empty instead of dereferencing null.
- Cache a key hash through map insertion and double bucket count on rehash
  (with overflow protection), reducing repeat hashing and the number of full
  rebuilds.
- Keep insertion workloads observable in optimized profiling builds, and add
  regression coverage for cross-container deque iterator comparisons, deque
  front wrapping from a non-power-of-two initial capacity, and empty `any`
  copies.

## Before / after measurements

Measurements were taken with GCC 14.2, CMake `Release` (`-O3`), without
sanitizers, on the same machine. Each benchmark uses two warmups and eleven
samples; values below are the reported median ns/op. The baseline used the
same corrected profiling harness as the optimized build so insertion state
was preserved in both runs. These are local benchmark results, not universal
performance guarantees.

| Workload | Before | After | Change |
|---|---:|---:|---:|
| Deque sequential iteration | 1.90 | 0.54 | 71.6% faster |
| Deque push_back, growing | 5.03 | 4.77 | 5.2% faster |
| Deque push_back, HackySTL reserved | 1.92 | 1.36 | 29.2% faster |
| Deque push_front, growing | 5.77 | 4.49 | 22.2% faster |
| Unordered-map insertion, unreserved | 86.55 | 54.98 | 36.5% faster |
| Unordered-map insertion, reserved | 20.22 | 19.66 | 2.8% faster |
| Any construct + cast | 18.11 | 12.99 | 28.3% faster |
| String-view find, hit | 21.87 | 6.02 | 72.5% faster |
| String-view find, miss | 12.22 | 8.03 | 34.3% faster |

## Remaining opportunities

The custom deque still pays for occasional full-buffer reallocations, so its
unreserved end-insertion timing remains above `std::deque`. The map's separate
vector per bucket also means many small allocations; doubling reduces rehash
work but does not remove that representation cost. `any` still heap-allocates
its holder for every value, unlike implementations that use small-object
storage, so allocation remains its main measured gap. Those larger structural
changes are intentionally left for a separate change because they affect
storage, ownership, and iterator guarantees more broadly.
