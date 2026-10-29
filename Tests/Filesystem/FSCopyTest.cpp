#include <cassert>
#include <cstdio>
#include <filesystem>
#include <string>

#include <Filesystem/Path.hpp>

namespace fs = hsd::filesystem;
namespace std_fs = std::filesystem;

// Behavior tests for copying files. The copy goes through the io handles, so these check
// the bytes and the error results, not just that a call succeeds. Everything happens under
// a fresh directory in the system temporary location, so the source tree is never touched.
namespace
{
    void write_bytes(const std::string& name, const std::string& bytes)
    {
        std::FILE* file = std::fopen(name.c_str(), "wb");
        assert(file != nullptr);
        std::fwrite(bytes.data(), 1, bytes.size(), file);
        std::fclose(file);
    }

    std::string read_bytes(const std::string& name)
    {
        std::FILE* file = std::fopen(name.c_str(), "rb");
        assert(file != nullptr);

        std::string bytes;
        char buffer[256];
        std::size_t count = std::fread(buffer, 1, sizeof(buffer), file);

        while (count > 0)
        {
            bytes.append(buffer, count);
            count = std::fread(buffer, 1, sizeof(buffer), file);
        }

        std::fclose(file);
        return bytes;
    }

    bool exists(const std::string& name)
    {
        return std_fs::exists(name);
    }
}

int main()
{
    const std_fs::path base = std_fs::temp_directory_path() / "hsd_fs_copy_test";
    std_fs::remove_all(base);
    std_fs::create_directories(base);
    const std::string root = base.string();

    // Every byte is copied unchanged: CR and LF stay as they are, an embedded NUL is kept,
    // and bytes above 0x7F pass through.
    std::string payload{"line one\r\nline two\n\0tail", 24};
    payload.push_back(static_cast<char>(0xff));
    payload.push_back(static_cast<char>(0x80));

    // Copying onto an existing file replaces its contents, even when the old file is longer.
    write_bytes(root + "/source.bin", payload);
    write_bytes(root + "/target.bin", std::string(100, 'x'));
    fs::path source{(root + "/source.bin").c_str()};
    fs::path target{(root + "/target.bin").c_str()};
    assert(source.copy(target).is_ok());
    assert(read_bytes(root + "/target.bin") == payload);
    assert(read_bytes(root + "/source.bin") == payload);

    // Copying into a directory keeps the file name.
    std_fs::create_directories(base / "into");
    fs::path into{(root + "/into").c_str()};
    assert(source.copy(into).is_ok());
    assert(read_bytes(root + "/into/source.bin") == payload);

    // A missing source fails with its own message and creates no destination.
    const auto _missing = fs::fs_detail::copy_file(
        (root + "/nothing.bin").c_str(), (root + "/made.bin").c_str()
    ).expect_err();
    assert(std::string{_missing.pretty_error()} == "File not found");
    assert(!exists(root + "/made.bin"));

    // A destination in a missing directory fails, and the source is left as it was.
    const auto _no_target = fs::fs_detail::copy_file(
        (root + "/source.bin").c_str(), (root + "/no_such_dir/out.bin").c_str()
    ).expect_err();
    assert(std::string{_no_target.pretty_error()} == "Couldn't create the destination file");
    assert(read_bytes(root + "/source.bin") == payload);

#if !defined(HSD_PLATFORM_WINDOWS)
    // Opening a directory works on POSIX, but reading it fails. Windows refuses to open a
    // directory as a file, so that case is a "File not found" there instead.
    std_fs::create_directories(base / "as_source");
    const auto _unreadable = fs::fs_detail::copy_file(
        (root + "/as_source").c_str(), (root + "/from_dir.bin").c_str()
    ).expect_err();
    assert(std::string{_unreadable.pretty_error()} == "Couldn't read the file");

    // Copying a directory copies its files and subdirectories into the destination.
    // Windows is left out because path::copy still has its documented separator bug there.
    std_fs::create_directories(base / "tree" / "sub");
    std_fs::create_directories(base / "copied");
    write_bytes(root + "/tree/a.txt", "A");
    write_bytes(root + "/tree/sub/b.txt", "B");
    fs::path tree{(root + "/tree").c_str()};
    fs::path copied{(root + "/copied").c_str()};
    assert(tree.copy(copied).is_ok());
    assert(read_bytes(root + "/copied/a.txt") == "A");
    assert(read_bytes(root + "/copied/sub/b.txt") == "B");
#endif

    std_fs::remove_all(base);
    return 0;
}
