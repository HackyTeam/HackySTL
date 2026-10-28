# Allocators {#allocators}

Allocator-aware HackySTL types separate raw storage from object lifetime.
Concrete allocators provide storage, while `hsd::allocator_traits` gives
containers one typed interface for allocation, construction, destruction, and
deallocation.

## Allocator contract

A reusable allocator only needs these two operations:

```cpp
template <typename T>
auto allocate(hsd::usize count)
    -> hsd::result<T*, hsd::allocator_detail::allocator_error>;

template <typename T>
auto deallocate(T* ptr, hsd::usize count)
    -> hsd::option_err<hsd::allocator_detail::allocator_error>;
```

`allocate<T>` returns uninitialized storage suitable for `count` objects of
type `T`. A successful zero-sized allocation returns `nullptr`. `deallocate`
accepts `nullptr` and does not destroy objects.

Use `allocator_traits<Allocator, T>` instead of invoking these operations
directly from an allocator-aware owner:

```cpp
using traits = hsd::allocator_traits<my_allocator, value_type>;

auto* ptr = traits::allocate(alloc, count).unwrap();
traits::construct(alloc, ptr, constructor_argument);

traits::destroy(alloc, ptr);
traits::deallocate(alloc, ptr, count).unwrap();
```

Specialize `allocator_traits` when an allocator needs custom construction,
destruction, or copy-selection behavior. Ordinary allocators can use the
default implementation unchanged.

## Built-in allocators

- `hsd::allocator` uses aligned `operator new`. It stores allocation metadata
  separately from the aligned object address, allowing compatible polymorphic
  pointers to release storage correctly.
- `hsd::mallocator` uses `malloc` and `free`. It rejects types requiring
  alignment greater than `max_align_t`.
- `hsd::buffered_allocator` uses a caller-owned byte buffer. It performs
  first-fit allocation, aligns each result for its requested type, and
  coalesces adjacent free blocks during deallocation.

## Stateful allocator ownership

`buffered_allocator` is a non-owning handle. The byte buffer must outlive every
allocator, container, and smart pointer using it. Copying or moving the
allocator creates another handle to the same block list.

Constructing a new `buffered_allocator` over a byte buffer resets that buffer's
allocation state. To share an initialized buffer, copy the existing allocator:

```cpp
alignas(64) hsd::uchar bytes[1024]{};
hsd::buffered_allocator first{bytes, sizeof(bytes)};
hsd::buffered_allocator second = first;
```

Allocator-aware containers keep an allocator paired with the storage it
created. Copies select an allocator through
`select_on_container_copy_construction`; assignments exchange complete
allocator-and-storage states. Moves leave the source valid and destructible.

## Container construction

A container constructor only requires `DefaultConstructible<Allocator>` when
it must create an allocator itself. Each such initialization form has a matching
overload whose final argument is `const Allocator&`. Operations on an existing
container use its stored allocator and do not require default construction.

This rule applies to `vector`, `deque`, `list`, `forward_list`, `heap_array`,
`basic_string`, `unordered_map`, and `basic_sstream`. Array constructors accept
an allocator alongside the source array. Each owning container exposes
`get_allocator`; `basic_sstream` forwards the vector accessor.

`unordered_map` propagates one allocator instance to its value vector, outer
bucket vector, and every nested bucket vector. Rehashing obtains the allocator
from the existing bucket storage rather than constructing a new instance.

## Adding allocator-aware owners

For contiguous storage, instantiate traits with the element type. For linked
storage, instantiate traits with the complete node type, not only its stored
value. Always destroy live objects before deallocating their storage, preferably
in reverse construction order.

Do not store allocation metadata at a fixed offset and then return the next
byte as an object pointer. Metadata size and object alignment are independent;
the final object address must be aligned explicitly.