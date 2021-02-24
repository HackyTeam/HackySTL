# Filesystem status support

## Filesystem status introduced in the library
Implemented in header [FStatus.hpp](../cpp/FStatus.hpp)

## Types available:

### Filesystem permission mask
#### Definition:
```cpp
enum class fs_perms_mask;
```

#### Member values:
| Name | Value |
| :--- | :---- |
| `owner_read` | `S_IRUSR(POSIX)` |
| `owner_write` | `S_IWUSR(POSIX)` |
| `owner_exec` | `S_IXUSR(POSIX)` |
| `owner_all` | `S_IRWXU(POSIX)` |
| `group_read` | `S_IRGRP(POSIX)` |
| `group_write` | `S_IWGRP(POSIX)` |
| `group_exec` | `S_IXGRP(POSIX)` |
| `group_all` | `S_IRWXG(POSIX)` |
| `others_read` | `S_IROTH(POSIX)` |
| `others_write` | `S_IWOTH(POSIX)` |
| `others_exec` | `S_IXOTH(POSIX)` |
| `others_all` | `S_IRWXO(POSIX)` |
| `all_perms` | `S_IRWXA(POSIX)` |

#### Member functions:
| Method | Arguments | Return type | Description |
| :----- | :-------- | :---------- | :---------- |
| `operator\|` | `const fs_perms_mask& lhs`, `const fs_perms_mask& rhs` | `fs_perms_mask` | Pipes bitwise the values into a new one |

### Filesystem permissions
#### Definition:
```cpp
struct fs_permissions;
```

#### Member values:
| Type | Name | Description |
| :--- | :--- | :---------- |
| `bool` | `can_owner_read` | File owner has read permission |
| `bool` | `can_owner_write` | File owner has write permission |
| `bool` | `can_owner_exec` | File owner has execute/search permission
| `bool` | `can_owner_do_everything` | File owner has read, write, and execute/search permissions |
| `bool` | `can_group_read` | The file's user group has read permission |
| `bool` | `can_group_write` | The file's user group has write permission |
| `bool` | `can_group_exec` | The file's user group has execute/search permission |
| `bool` | `can_group_do_everything` | The file's user group has read, write, and execute/search permissions |
| `bool` | `can_others_read` | Other users have read permission |
| `bool` | `can_others_write` | Other users have write permission |
| `bool` | `can_others_exec` | Other users have execute/search permission |
| `bool` | `can_others_do_everything` | Other users have read, write, and execute/search permissions |
| `bool` | `can_all_do_everything` | Everyone can read write and execute the file |

### Filesystem status
#### Definition:
```cpp
class fs_status;
```

#### Member functions:
| Method | Arguments | Return type | Description |
| :----- | :-------- | :---------- | :---------- |
| `fs_status` | `const char* pathname` | `/*compiler defined*/` | Initialized the underlying status type and checks if the file/directory exists |
| `operator==` | `const fs_status& rhs` | `bool` | Checks if 2 statuses have the same attributes |
| `exists` | `N/A` | `bool` | Returns `true` is the file/directory exists or `false` otherwise |
| `is_directory` | `N/A` | `Result<bool, runtime_error>` | If it exists, checks if it is a directory, otherwise it returns an error |
| `is_character` | `N/A` | `Result<bool, runtime_error>` | If it exists, checks if it is a character file, otherwise it returns an error |
| `is_block_file` | `N/A` | `Result<bool, runtime_error>` | If it exists, checks if it is a block file, otherwise it returns an error |
| `is_regular_file` | `N/A` | `Result<bool, runtime_error>` | If it exists, checks if it is a file without any special properties, otherwise it returns an error |
| `is_fifo_file` | `N/A` | `Result<bool, runtime_error>` | If it exists, checks if it is a First In First Out file, otherwise it returns an error |
| `is_symlink` | `N/A` | `Result<bool, runtime_error>` | If it exists, checks if it is a link to another file/directory, otherwise it returns an error |
| `is_socket` | `N/A` | `Result<bool, runtime_error>` | If it exists, checks if it is a socket file, otherwise it returns an error |
| `permissions` | `N/A` | `Result<fs_permissions, runtime_error>` | If it exists, returns its permissions, otherwise it returns an error |
| `size` | `N/A` | `Result<usize, runtime_error>` | If it exists, it gets the size of the file, otherwise it returns an error |