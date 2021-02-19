# Allocators support

## Multiple allocation strategies introduced in the library
Implemented in header [Allocator.hpp](../cpp/Allocator.hpp)

## Allocators:

### Buffered allocator
#### Definition:
```cpp
template <typename Type>
class buffered_allocator;
```

#### Description:
Allocates a block of memory in a buffer with a defined size by using block structure with the definition of:
```cpp
struct block 
{
    usize size{};
    bool in_use{};
    uchar data[]; // flexible array for accessing the data
};
```

#### Public members:
| Member | Value/Type |
| :----- | :--------- |
| pointer_type | `Type*` |
| value_type | `Type` |

#### Member functions:
| Method | Arguments | Return type | Description |
| :----- | :-------- | :---------- | :---------- |
| `buffered_allocator` | `uchar* buf`, `usize size` | `/*implementation defined*/` | Sets the internal buffer for allocations and size |
| `buffered_allocator` | `const buffered_allocator& other` | `/*implementation defined*/` | Copy constructor |
| `buffered_allocator` | `buffered_allocator&& other` | `/*implementation defined*/` | Move constructor |
| `operator=` | `const buffered_allocator<UType>& rhs` | `buffered_allocator&` | Copy attribution |
| `allocate` | `usize size` | `Result<Type*, allocator_error>` | Allocates a block with a size of `sizeof(Type) * size` inside the buffer |
| `deallocate` | `Type* ptr`, `usize` | `Result<void, allocator_error>` | Deallocates a block with a size defined is its structured inside the buffer |
| `print_buffer` | `N/A` | `void` | If it is compiled in debug mode this method is enabled and prints the buffer in a matrix configuration |

### Heap allocator
#### Definition:
```cpp
template <typename Type>
class allocator;
```

#### Public members:
| Member | Value/Type |
| :----- | :--------- |
| pointer_type | `Type*` |
| value_type | `Type` |

#### Description:
Allocates memory on the heap allocation `::operator new`

#### Member functions:
| Method | Arguments | Return type | Description |
| :----- | :-------- | :---------- | :---------- |
| `allocator` | `N/A` | `/*implementation defined*/` | Sets the type size and the alignment according to `Type` |
| `allocator` | `const allocator& other` | `/*implementation defined*/` | Copy constructor |
| `allocator` | `allocator&& other` | `/*implementation defined*/` | Move constructor |
| `operator=` | `const allocator<UType>& rhs` | `allocator&` | Copy attribution |
| `allocate` | `usize size` | `Result<Type*, allocator_error>` | Allocates a block with a size of `type_size * size` and with an `alignment` on the heap |
| `deallocate` | `Type* ptr`, `usize size` | `Result<void, allocator_error>` | Deallocates a block from the heap using the `size`, `type_size` and `alignment` as helping parameters |

### Constexpr allocator
#### Definition:
```cpp
template < typename Type, usize MaxSize >
class constexpr_allocator;
```

#### Public members:
| Member | Value/Type |
| :----- | :--------- |
| pointer_type | `stack_array<Type, MaxSize>` |
| value_type | `Type` |

#### Description:
It is a wrapper around `stack_array` which can be used at compile time

#### Member functions:
| Method | Arguments | Return type | Description |
| :----- | :-------- | :---------- | :---------- |
| `constexpr_allocator` | `N/A` | `/*implementation defined*/` | Initialize the internal array |
| `constexpr_allocator` | `const constexpr_allocator&` | `/*implementation defined*/` | Copy constructor (deleted) |
| `constexpr_allocator` | `constexpr_allocator&&` | `/*implementation defined*/` | Move constructor (deleted) |
| `limit` | `N/A` | `usize` | Returns the `MaxSize` template parameter |