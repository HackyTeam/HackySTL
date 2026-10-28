/*
MIT License
Copyright (c) 2019 win32ports
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/// @file _dirent_win32.h
/// @brief Windows compatibility declarations for directory traversal.
/// @details Supplies the dirent-style structures, constants, and functions
/// required by the filesystem layer when building against Win32 APIs.

#pragma once

#ifndef __DIRENT_H_9DE6B42C_8D0C_4D31_A8EF_8E4C30E6C46A__
#define __DIRENT_H_9DE6B42C_8D0C_4D31_A8EF_8E4C30E6C46A__

#ifndef _WIN32

#pragma message("this dirent.h implementation is for Windows only!")

#else /* _WIN32 */

#ifdef __cplusplus
#define HSD_DIRENT_UNUSED [[maybe_unused]]
#else
/* MSVC has no __attribute__. GCC and MinGW need it to silence unused-function warnings. */
#ifdef _MSC_VER
#define HSD_DIRENT_UNUSED
#else
#define HSD_DIRENT_UNUSED HSD_DIRENT_UNUSED
#endif /* _MSC_VER */
#endif /* __cplusplus */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <sys/types.h>
#include <stdint.h>
#include <errno.h>

#define _WINSOCKAPI_
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <shlwapi.h>

#ifdef _MSC_VER
#pragma comment(lib, "Shlwapi.lib")
#endif

#ifndef NAME_MAX
#define NAME_MAX 260
#endif /* NAME_MAX */

#ifndef DT_UNKNOWN
#define DT_UNKNOWN 0
#endif /* DT_UNKNOWN */

#ifndef DT_FIFO
#define DT_FIFO 1
#endif /* DT_FIFO */

#ifndef DT_CHR
#define DT_CHR 2
#endif /* DT_CHR */

#ifndef DT_DIR
#define DT_DIR 4
#endif /* DT_DIR */

#ifndef DT_BLK
#define DT_BLK 6
#endif /* DT_BLK */

#ifndef DT_REG
#define DT_REG 8
#endif /* DT_REF */

#ifndef DT_LNK
#define DT_LNK 10
#endif /* DT_LNK */

#ifndef DT_SOCK
#define DT_SOCK 12
#endif /* DT_SOCK */

#ifndef DT_WHT
#define DT_WHT 14
#endif /* DT_WHT */

#ifndef _DIRENT_HAVE_D_NAMLEN
#define _DIRENT_HAVE_D_NAMLEN 1
#endif /* _DIRENT_HAVE_D_NAMLEN */

#ifndef _DIRENT_HAVE_D_RECLEN
#define _DIRENT_HAVE_D_RECLEN 1
#endif /* _DIRENT_HAVE_D_RECLEN */

#ifndef _DIRENT_HAVE_D_OFF
#define _DIRENT_HAVE_D_OFF 1
#endif /* _DIRENT_HAVE_D_OFF */

#ifndef _DIRENT_HAVE_D_TYPE 
#define _DIRENT_HAVE_D_TYPE 1
#endif /* _DIRENT_HAVE_D_TYPE  */

#ifndef NTFS_MAX_PATH
#define NTFS_MAX_PATH 32768
#endif /* NTFS_MAX_PATH */

#ifndef FSCTL_GET_REPARSE_POINT
#define FSCTL_GET_REPARSE_POINT 0x900a8
#endif /* FSCTL_GET_REPARSE_POINT */

#ifndef FILE_NAME_NORMALIZED
#define FILE_NAME_NORMALIZED 0
#endif /* FILE_NAME_NORMALIZED */

/// Opaque handle to an open directory stream (implemented as `struct __dir*`).
typedef void* DIR;

/// POSIX-style inode identifier synthesized from Windows file-id information.
typedef struct
{
	/// NTFS volume serial number.
	unsigned long long serial;
	/// 128-bit NTFS/ReFS file identifier (or 64-bit legacy id, right-aligned).
	unsigned char fileid[16];
} __ino_t;

/// POSIX-compatible directory entry, populated by `readdir()`.
struct dirent
{
	/// Inode identifier of the entry.
	__ino_t d_ino;
	/// Offset of the entry (unused, always 0).
	off_t d_off;
	/// Length in bytes of this record (always `sizeof(struct dirent)`).
	unsigned short d_reclen;
	/// Length of the file name, in wide characters.
	unsigned char d_namelen;
	/// Entry type: one of the `DT_*` constants above.
	unsigned char d_type;
	/// Null-terminated, UTF-8 encoded file name.
	char d_name[NAME_MAX];
};

/// Internal directory stream state backing the opaque `DIR` handle.
struct __dir
{
	/// Heap-allocated array of directory entries collected up front.
	struct dirent* entries;
	/// Native handle opened on the directory itself (for `dirfd()`).
	intptr_t fd;
	/// Total number of valid entries in `entries`.
	long int count;
	/// Index of the next entry to return from `readdir()`.
	long int index;
};

///
/// @brief Closes a directory stream opened by `opendir()`/`_wopendir()`/
/// `fdopendir()`, releasing its native handle and entry buffer.
/// 
/// @param dirp - The directory stream to close.
/// @return int - 0 on success, or -1 with `errno` set to `EBADF` if `dirp`
/// is `NULL`.
///
static int closedir(DIR* dirp)
{
	struct __dir* data = NULL;
	
	if (dirp == NULL)
	{
		errno = EBADF;
		return -1;
	}

	data = (struct __dir*) dirp;
	
	CloseHandle((HANDLE)data->fd);
	free(data->entries);
	free(data);
	
	return 0;
}

///
/// @brief Sets `errno` (via `_set_errno` under MSVC) to `value`.
/// 
/// @param value - The error code to set.
///
static void __seterrno(int value)
{
#ifdef _MSC_VER
	_set_errno(value);
#else /* _MSC_VER */
	errno = value;
#endif /* _MSC_VER */
}

///
/// @brief Checks whether `name` is a reparse point tagged as a symbolic
/// link, using `FSCTL_GET_REPARSE_POINT`.
/// 
/// @param name - Wide-character path to check.
/// @param buffer - Scratch buffer of at least
/// `MAXIMUM_REPARSE_DATA_BUFFER_SIZE` bytes, filled with the reparse point
/// data on success.
/// @return int - Nonzero if `name` is a symlink reparse point, 0 if it is
/// not (or could not be opened/queried).
///
static int __islink(const wchar_t * name, char * buffer)
{
	DWORD io_result = 0;
	DWORD bytes_returned = 0;
	HANDLE hFile = CreateFileW(name, 0, 0, NULL, OPEN_EXISTING,
		FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, 0);
	
	if (hFile == INVALID_HANDLE_VALUE)
		return 0;

	io_result = DeviceIoControl(hFile, FSCTL_GET_REPARSE_POINT, NULL, 0,
		buffer, MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &bytes_returned, NULL);

	CloseHandle(hFile);

	if (io_result == 0)
		return 0;

	return ((REPARSE_GUID_DATA_BUFFER*)buffer)->ReparseTag == IO_REPARSE_TAG_SYMLINK;
}

#pragma pack(push, 1)

/// 128-bit file identifier as reported by `GetFileInformationByHandleEx`.
typedef struct dirent_FILE_ID_128
{
	/// Raw 128-bit identifier bytes.
	BYTE Identifier[16];
}
dirent_FILE_ID_128;

/// Volume serial number plus 128-bit file id, identifying a file uniquely.
typedef struct _dirent_FILE_ID_INFO
{
	/// Serial number of the volume the file resides on.
	ULONGLONG VolumeSerialNumber;
	/// 128-bit identifier of the file within its volume.
	dirent_FILE_ID_128 FileId;
}
dirent_FILE_ID_INFO;

#pragma pack(pop)

/// Local redeclaration of the `FileIdInfo` class value (18), used instead of
/// the SDK's `FILE_INFO_BY_HANDLE_CLASS` enumerator to avoid depending on
/// newer Windows SDK headers.
typedef enum dirent_FILE_INFO_BY_HANDLE_CLASS
{ dirent_FileIdInfo = 18 }
dirent_FILE_INFO_BY_HANDLE_CLASS;

///
/// @brief Computes a POSIX-style inode identifier for `name`, preferring
/// the 128-bit `GetFileInformationByHandleEx(FileIdInfo)` result and
/// falling back to the legacy `GetFileInformationByHandle` volume
/// serial/file-index pair (packed into the last 8 bytes of `fileid`) if the
/// former is unavailable.
/// 
/// @param name - Wide-character path of the file to query.
/// @return __ino_t - The computed identifier, or a zeroed value if the
/// file could not be opened or both info queries fail.
///
static __ino_t __inode(const wchar_t* name)
{
	__ino_t value = { 0 };
	BOOL result;
	dirent_FILE_ID_INFO fileid;
	BY_HANDLE_FILE_INFORMATION info;


	HANDLE hFile = CreateFileW(name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	
	if (hFile == INVALID_HANDLE_VALUE)
		return value;

	result = GetFileInformationByHandleEx(hFile, (FILE_INFO_BY_HANDLE_CLASS)dirent_FileIdInfo, &fileid, sizeof(fileid));
	
    if (result == true)
	{
		value.serial = fileid.VolumeSerialNumber;
		memcpy(value.fileid, fileid.FileId.Identifier, 16);
	}
	else
	{
		result = GetFileInformationByHandle(hFile, &info);
		if(result)
		{
			value.serial = info.dwVolumeSerialNumber;
			memcpy(value.fileid + 8, &info.nFileIndexHigh, 4);
			memcpy(value.fileid + 12, &info.nFileIndexLow, 4);
		}
	}
	
	CloseHandle(hFile);
	return value;
}

///
/// @brief Enumerates all entries of the directory named by `wname` up
/// front (via repeated `FindFirstFileW`/`FindNextFileW`) and stores them in
/// a heap-allocated `struct __dir`. `wname` must already contain the
/// `\\?\`-prefixed buffer produced by `__get_buffer()`, sized for at least
/// `NTFS_MAX_PATH + NAME_MAX + 8` wide characters; this function mutates
/// the buffer in place (appending/removing the `\*.*` search suffix and
/// stripping the `\\?\` prefix for relative paths) and does not take
/// ownership of it.
/// 
/// @param wname - Mutable wide-character path buffer, as produced by
/// `__get_buffer()` plus the encoded path.
/// @param size - Length in wide characters of the path currently stored in
/// `wname` (including the `\\?\` prefix and null terminator).
/// @return DIR* - A new directory stream, or `NULL` with `errno` set to
/// `ENOENT` (directory not found) or `ENOMEM` (allocation failure).
///
static DIR* __internal_opendir(wchar_t* wname, int size)
{
	struct __dir* data = NULL;
	struct dirent* tmp_entries = NULL;
	
    char default_char = '?';
	
    const wchar_t* prefix = L"\\\\?\\";
	const wchar_t* suffix = L"\\*.*";
	const int extra_prefix = 4; /* use prefix "\\?\" to handle long file names */
	const int extra_suffix = 4; /* use suffix "\*.*" to find everything */
	
    WIN32_FIND_DATAW w32fd = { 0 };
	HANDLE hFindFile = INVALID_HANDLE_VALUE;
	
    const int grow_factor = 2;
	char* buffer = NULL;

	BOOL relative = PathIsRelativeW(wname + extra_prefix);

	memcpy(wname + size - 1, suffix, sizeof(wchar_t) * extra_suffix);
	wname[size + extra_suffix - 1] = 0;

	if (relative == true)
    {
		wname += extra_prefix;
		size -= extra_prefix;
	}

	hFindFile = FindFirstFileW(wname, &w32fd);
	
    if (INVALID_HANDLE_VALUE == hFindFile)
	{
		__seterrno(ENOENT);
		return NULL;
	}

	data = (struct __dir*) malloc(sizeof(struct __dir));
	
    if (data == NULL)
    {
		goto out_of_memory;
    }
	
    wname[size - 1] = 0;
	
    data->fd = (intptr_t)CreateFileW(wname, 0, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0);
	
    wname[size - 1] = L'\\';
	
    data->count = 16;
	data->index = 0;
	data->entries = (struct dirent*) malloc(sizeof(struct dirent) * data->count);
	
    if (data->entries == NULL)
    {
		goto out_of_memory;
    }

    buffer = (char*)malloc(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
	
    if (buffer == NULL)
    {
		goto out_of_memory;
    }
	
    do
	{
		WideCharToMultiByte(CP_UTF8, 0, w32fd.cFileName, -1, data->entries[data->index].d_name, NAME_MAX, &default_char, NULL);

		memcpy(wname + size, w32fd.cFileName, sizeof(wchar_t) * NAME_MAX);

		if (((w32fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT) && __islink(wname, buffer))
		{
            data->entries[data->index].d_type = DT_LNK;
        }
		else if ((w32fd.dwFileAttributes & FILE_ATTRIBUTE_DEVICE) == FILE_ATTRIBUTE_DEVICE)
        {
            data->entries[data->index].d_type = DT_CHR;
        }
        else if ((w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		{
            data->entries[data->index].d_type = DT_DIR;
        }
        else
		{
            data->entries[data->index].d_type = DT_REG;
        }

		data->entries[data->index].d_ino = __inode(wname);
		data->entries[data->index].d_reclen = sizeof(struct dirent);
		data->entries[data->index].d_namelen = (unsigned char)wcslen(w32fd.cFileName);
		data->entries[data->index].d_off = 0;

		if (++data->index == data->count)
        {
			tmp_entries = (struct dirent*) realloc(data->entries, sizeof(struct dirent) * data->count * grow_factor);

			if (tmp_entries == NULL)
				goto out_of_memory;

			data->entries = tmp_entries;
			data->count *= grow_factor;
		}
	}
	while (FindNextFileW(hFindFile, &w32fd) != 0);

	free(buffer);
	FindClose(hFindFile);

	data->count = data->index;
	data->index = 0;
	return (DIR*)data;

out_of_memory:

	if (data != NULL)
	{
		if (INVALID_HANDLE_VALUE != (HANDLE)data->fd)
        {
			CloseHandle((HANDLE)data->fd);
        }

    	free(data->entries);
	}
	
    free(buffer);
	free(data);
	
    if (INVALID_HANDLE_VALUE != hFindFile)
	{
        FindClose(hFindFile);
    }
	
    __seterrno(ENOMEM);
	return NULL;
}

///
/// @brief Allocates a wide-character path buffer large enough for
/// `__internal_opendir()`, pre-filled with the `\\?\` long-path prefix.
/// 
/// @return wchar_t* - A heap-allocated buffer the caller must `free()`, or
/// `NULL` on allocation failure.
///
static wchar_t* __get_buffer()
{
	wchar_t* name = (wchar_t*) malloc(sizeof(wchar_t) * (NTFS_MAX_PATH + NAME_MAX + 8));
	
    if (name)
    {
		memcpy(name, L"\\\\?\\", sizeof(wchar_t) * 4);
    }
	
    return name;
}

///
/// @brief Opens a directory stream for the UTF-8 encoded path `name`.
/// 
/// @param name - UTF-8 encoded path of the directory to open.
/// @return DIR* - A new directory stream, or `NULL` with `errno` set to
/// `ENOMEM` on allocation failure, or as set by `__internal_opendir()`/
/// `MultiByteToWideChar()` on other failures.
///
static DIR* opendir(const char* name)
{
	DIR* dirp = NULL;
	
    wchar_t* wname = __get_buffer();
	int size = 0;
	
    if (wname == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}
	
    size = MultiByteToWideChar(CP_UTF8, 0, name, -1, wname + 4, NTFS_MAX_PATH);
	
    if (0 == size)
	{
		free(wname);
		return NULL;
	}
	
    dirp = __internal_opendir(wname, size + 4);
	free(wname);
	
    return dirp;
}

///
/// @brief Wide-character variant of `opendir()`.
/// 
/// @param name - Wide-character path of the directory to open.
/// @return DIR* - A new directory stream, or `NULL` with `errno` set to
/// `ENOMEM` on allocation failure, or unset (stale) if `name` exceeds
/// `NTFS_MAX_PATH` characters.
///
static DIR* _wopendir(const wchar_t* name) HSD_DIRENT_UNUSED;
static DIR* _wopendir(const wchar_t* name)
{
	DIR* dirp = NULL;
	wchar_t* wname = __get_buffer();
	int size = 0;
	
    if (!wname)
	{
		errno = ENOMEM;
		return NULL;
	}
	
    size = (int)wcslen(name);
	
    if (size > NTFS_MAX_PATH)
	{
		free(wname);
		return NULL;
	}
	
    memcpy(wname + 4, name, sizeof(wchar_t) * (size + 1));
	dirp = __internal_opendir(wname, size + 5);
	free(wname);
	
    return dirp;
}

///
/// @brief Opens a directory stream from an already-open native directory
/// handle, by resolving its final path via `GetFinalPathNameByHandleW()`.
/// 
/// @param fd - Native handle of an already-open directory.
/// @return DIR* - A new directory stream, or `NULL` with `errno` set to
/// `ENOMEM` on allocation failure or `ENOTDIR` if the path could not be
/// resolved from `fd`.
///
static DIR* fdopendir(intptr_t fd) HSD_DIRENT_UNUSED;
static DIR* fdopendir(intptr_t fd)
{
	DIR* dirp = NULL;
	wchar_t* wname = __get_buffer();

	int size = 0;
	
    if (!wname)
	{
		errno = ENOMEM;
		return NULL;
	}
	
    size = GetFinalPathNameByHandleW((HANDLE) fd, wname + 4, NTFS_MAX_PATH, FILE_NAME_NORMALIZED);
	
    if (0 == size)
	{
		free(wname);
		errno = ENOTDIR;
	
    	return NULL;
	}
	
    dirp = __internal_opendir(wname, size + 5);
	free(wname);
	
    return dirp;
}

///
/// @brief Returns the next entry of the directory stream and advances its
/// internal position. The returned pointer refers to storage owned by
/// `dirp` and is invalidated by `closedir()`.
/// 
/// @param dirp - The directory stream to read from.
/// @return struct dirent* - The next entry, or `NULL` at end-of-stream or
/// if `dirp` is `NULL` (with `errno` set to `EBADF` in the latter case).
///
static struct dirent* readdir(DIR* dirp)
{
	struct __dir* data = (struct __dir*) dirp;
	
    if (data == NULL)
    {
		errno = EBADF;
		return NULL;
	}
	
    if (data->index < data->count)
	{
		return &data->entries[data->index++];
	}
	
    return NULL;
}

///
/// @brief Reentrant variant of `readdir()`, copying the next entry into a
/// caller-supplied buffer instead of returning an internal pointer.
/// 
/// @param dirp - The directory stream to read from.
/// @param entry - Buffer to receive the next entry, or `NULL` to skip copying.
/// @param result - Set to `entry` on success, or `NULL` at end-of-stream.
/// @return int - 0 on success (including end-of-stream), or `EBADF` if
/// `dirp` is `NULL`.
///
static int readdir_r(DIR* dirp, struct dirent* entry, struct dirent** result) HSD_DIRENT_UNUSED;
static int readdir_r(DIR* dirp, struct dirent* entry, struct dirent** result)
{
	struct __dir* data = (struct __dir*) dirp;
	
    if (data == NULL)
    {
		return EBADF;
	}

	if (data->index < data->count)
	{
		if (entry)
        {
			memcpy(entry, &data->entries[data->index++], sizeof(struct dirent));
        }

		if (result)
        {
			*result = entry;
        }
	}
	else if (result)
    {
		*result = NULL;
    }

	return 0;
}

///
/// @brief Repositions the directory stream to entry index `offset`.
/// Ignored (no-op) if `dirp` is `NULL` or `offset` is not less than the
/// total entry count.
/// 
/// @param dirp - The directory stream to reposition.
/// @param offset - Zero-based entry index to seek to.
///
static void seekdir(DIR* dirp, long int offset)
{
	if (dirp)
	{
		struct __dir* data = (struct __dir*) dirp;
		data->index = (offset < data->count) ? offset : data->index;
	}
}

///
/// @brief Resets the directory stream to its first entry.
/// 
/// @param dirp - The directory stream to rewind.
///
static void rewinddir(DIR* dirp) HSD_DIRENT_UNUSED;
static void rewinddir(DIR* dirp)
{
	seekdir(dirp, 0);
}

///
/// @brief Gets the current stream position, for use with `seekdir()`.
/// Pre-existing bug: this returns `data->count` (the total number of
/// entries) instead of `data->index` (the current read position), so it
/// does not actually report the stream's current position; a later
/// `seekdir(dirp, telldir(dirp))` will seek to the end rather than restore
/// the original position.
/// 
/// @param dirp - The directory stream to query.
/// @return long int - The (incorrect) position value described above, or
/// -1 with `errno` set to `EBADF` if `dirp` is `NULL`.
///
static long int telldir(DIR* dirp) HSD_DIRENT_UNUSED;
static long int telldir(DIR* dirp)
{
	if (dirp == NULL)
    {
		errno = EBADF;
		return -1;
	}
	
    return ((struct __dir*)dirp)->count;
}

///
/// @brief Gets the native handle opened on the directory itself.
/// 
/// @param dirp - The directory stream to query.
/// @return intptr_t - The native handle, or -1 with `errno` set to
/// `EINVAL` if `dirp` is `NULL`.
///
static intptr_t dirfd(DIR * dirp) HSD_DIRENT_UNUSED;
static intptr_t dirfd(DIR * dirp)
{
	if (dirp == NULL)
    {
		errno = EINVAL;
		return -1;
	}
	
    return ((struct __dir*)dirp)->fd;
}

///
/// @brief Reads and sorts the entries of the directory named by `dirp`,
/// optionally filtering them, mirroring POSIX `scandir()`.
/// Pre-existing bug: in both out-of-memory cleanup paths, the loop meant to
/// free previously allocated entries calls `free(entries[index])` instead
/// of `free(entries[i])` — it repeatedly frees the same (out-of-range, at
/// that point unallocated) slot instead of each individually allocated
/// entry, leaking every real entry and risking an invalid `free()`.
/// Also, `compar` (a two-argument `int(const struct dirent**, const struct
/// dirent**)` comparator) is cast to the `int(const void*, const void*)`
/// comparator that plain `qsort()` calls. The element pointers `qsort` passes
/// are `struct dirent**`, so the two types agree in calling convention.
/// 
/// @param dirp - Path of the directory to scan.
/// @param namelist - Set to a heap-allocated, NULL-terminated array of
/// heap-allocated `dirent*` entries; the caller is responsible for freeing
/// each entry and the array itself.
/// @param filter - Optional predicate; entries for which it returns 0 are
/// excluded. Pass `NULL` to include everything.
/// @param compar - Comparator used to sort the resulting entries.
/// @return int - The number of entries on success, or -1 with `errno` set
/// to `ENOENT` (directory could not be opened) or `ENOMEM` (allocation
/// failure).
///
static int scandir(const char* dirp, struct dirent*** namelist,
	int (*filter)(const struct dirent*),
	int (*compar)(const struct dirent**, const struct dirent**)) HSD_DIRENT_UNUSED;
static int scandir(const char* dirp, struct dirent*** namelist,
	int (*filter)(const struct dirent*),
	int (*compar)(const struct dirent**, const struct dirent**))
{
	struct dirent ** entries = NULL, ** tmp_entries = NULL;
	long int i = 0, index = 0, count = 16;
	
    DIR * d = opendir(dirp);
	struct __dir* data = (struct __dir*) d;
	
    if (!data)
    {
		closedir(d);
		__seterrno(ENOENT);
	
    	return -1;
	}
	
    entries = (struct dirent**) malloc(sizeof(struct dirent*) * count);
	
    if (!entries)
	{
		closedir(d);
		__seterrno(ENOMEM);
		return -1;
	}

	for (i = 0; i < data->count; ++i)
	{
		if (!filter || filter(&data->entries[i]))
		{
			entries[index] = (struct dirent*) malloc(sizeof(struct dirent));

			if (!entries[index])
			{
				closedir(d);

				for (i = 0; i < index; ++i)
                {
					free(entries[index]);
                }

				free(entries);
				__seterrno(ENOMEM);

				return -1;
			}

			memcpy(entries[index], &data->entries[i], sizeof(struct dirent));

			if (++index == count)
			{
				tmp_entries = (struct dirent**)realloc(entries, sizeof(struct dirent*) * count * 2);

				if (!tmp_entries)
				{
					closedir(d);

					for (i = 0; i < index; ++i)
                    {
						free(entries[index - 1]);
                    }

					free(entries);
					__seterrno(ENOMEM);

					return -1;
				}

				entries = tmp_entries;
				count *= 2;
			}
		}
	}

	qsort(entries, index, sizeof(struct dirent*), (int (*)(const void*, const void*))compar);
	entries[index] = NULL;

	if (namelist)
    {
		*namelist = entries;
    }

	closedir(d);
	return 0;
}

///
/// @brief Comparator for `scandir()` sorting entries by name using the
/// current locale's collation order (`strcoll`).
/// 
/// @param a - Pointer to a `const struct dirent*` to compare.
/// @param b - Pointer to a `const struct dirent*` to compare.
/// @return int - Negative, zero, or positive per `strcoll()`, or 0 if
/// either argument is `NULL`.
///
static int alphasort(const void* a, const void* b) HSD_DIRENT_UNUSED;
static int alphasort(const void* a, const void* b)
{
	struct dirent** dira = (struct dirent**)a, **dirb = (struct dirent**)b;

	if (a == NULL || b == NULL)
    {
		return 0;
    }

	return strcoll((*dira)->d_name, (*dirb)->d_name);
}

///
/// @brief Intended as a version-aware string comparator, but delegates to
/// `alphasort()`.
/// Pre-existing bug: `s1`/`s2` are plain `const char*` strings, yet
/// `alphasort()` treats its arguments as `const struct dirent**` and
/// dereferences them twice (`(*dira)->d_name`) — this reinterprets the
/// string pointers as pointers-to-pointers-to-`dirent` and reads through
/// them, which is a type-confusion bug causing undefined behavior
/// (typically a crash or garbage comparison) whenever this function is
/// actually called.
/// 
/// @param s1 - First string (misinterpreted internally, see bug note).
/// @param s2 - Second string (misinterpreted internally, see bug note).
/// @return int - The (unreliable) result of the miscast `alphasort()` call.
///
static int __strverscmp(const char* s1, const char* s2) HSD_DIRENT_UNUSED;
static int __strverscmp(const char* s1, const char* s2)
{
	return alphasort(s1, s2);
}

///
/// @brief Comparator for `scandir()` intended to sort entries by name in
/// "version" order (i.e. via `__strverscmp()`).
/// Pre-existing bug: extracts `d_name` (a `char*`) from each entry and
/// forwards it to `__strverscmp()`, which misinterprets those `char*`
/// values as `struct dirent**` (see the bug note on `__strverscmp()`) —
/// this comparator is therefore unsafe to actually use.
/// 
/// @param a - Pointer to a `const struct dirent*` to compare.
/// @param b - Pointer to a `const struct dirent*` to compare.
/// @return int - The (unreliable) result of the underlying `__strverscmp()`
/// call, or 0 if either argument is `NULL`.
///
static int versionsort(const void* a, const void* b) HSD_DIRENT_UNUSED;
static int versionsort(const void* a, const void* b)
{
	struct dirent** dira = (struct dirent**)a, ** dirb = (struct dirent**)b;
	
    if (a == NULL || b == NULL)
    {
		return 0;
    }

	return __strverscmp((*dira)->d_name, (*dirb)->d_name);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _WIN32 */

#endif /* __DIRENT_H_9DE6B42C_8D0C_4D31_A8EF_8E4C30E6C46A__ */

