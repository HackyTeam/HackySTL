#include <cassert>
#include <cstdio>
#include <filesystem>
#include <string>

#include <Filesystem/Path.hpp>

#if defined(__unix__) || defined(__APPLE__)
#include <sys/stat.h>
#endif

namespace fs = hsd::filesystem;
namespace std_fs = std::filesystem;

// The error and special-case paths of the file-system helpers: a listing or a permission change
// on a path that is missing, a rename into a directory that does not exist, copies between a
// directory and a file, a named pipe that is neither a file nor a directory, a name with a
// separator given to create_directory, and a write that the device refuses. Everything happens
// under a fresh directory in the system temporary location, so the source tree is never touched.
namespace
{
    void write_text_file(const std::string& name)
    {
        std::FILE* file = std::fopen(name.c_str(), "wb");
        assert(file != nullptr);
        std::fputs("data", file);
        std::fclose(file);
    }

    bool exists(const std::string& name)
    {
        return std_fs::exists(name);
    }
}

int main()
{
    const std::string root = (std_fs::temp_directory_path() / "hsd_fs_edge_test").string();
    std_fs::remove_all(root);
    std_fs::create_directories(root);

    write_text_file(root + "/a.txt");
    fs::path top{root.c_str()};
    assert(top.create_directory("sub").is_ok());

    // A listing of a path that does not exist is an error, and a listing of a directory is not.
    fs::path missing{(root + "/no_such_directory").c_str()};
    assert(!missing.list().is_ok());
    assert(top.list().is_ok());

    // Changing the permissions of a path that does not exist is an error.
    assert(!missing.change_permissions(fs::fs_perms_mask::owner_read).is_ok());

    // A rename into a directory that does not exist fails, and the file keeps its name.
    fs::path first{(root + "/a.txt").c_str()};
    assert(!first.rename("no_such_dir/b.txt").is_ok());
    assert(exists(root + "/a.txt"));

    // A file cannot be renamed onto a directory, and the file keeps its name.
    assert(top.create_directory("taken").is_ok());
    assert(!first.rename("taken").is_ok());
    assert(exists(root + "/a.txt"));

    // A relative name with no directory part renames in the current directory.
    write_text_file("hsd_fs_edge_plain.txt");
    fs::path plain{"hsd_fs_edge_plain.txt"};
    assert(plain.rename("hsd_fs_edge_renamed.txt").is_ok());
    assert(exists("hsd_fs_edge_renamed.txt") && !exists("hsd_fs_edge_plain.txt"));
    std::remove("hsd_fs_edge_renamed.txt");

    // A directory cannot be copied over a file, but a file can be copied into a directory.
    fs::path sub{(root + "/sub").c_str()};
    assert(!sub.copy(first).is_ok());
    assert(first.copy(sub).is_ok());
    assert(exists(root + "/sub/a.txt"));

    // A directory copies its entries into another directory that exists.
    fs::path source_dir{(root + "/source").c_str()};
    assert(top.create_directory("source").is_ok());
    write_text_file(root + "/source/inner.txt");
    assert(top.create_directory("target").is_ok());
    fs::path target_dir{(root + "/target").c_str()};
    assert(source_dir.copy(target_dir).is_ok());
    assert(exists(root + "/target/inner.txt"));

    // The extension of a file is read from its own name, not from a directory name with a dot.
    assert(top.create_directory("dot.").is_ok());
    write_text_file(root + "/dot./plain");
    assert(fs::path{(root + "/dot./plain").c_str()}.extension().length() == 0);

    // A name with a separator in it cannot be created as one directory.
    assert(!top.create_directory("nested/child").is_ok());

    // The status of a path that does not exist reports that, for every kind of question.
    assert(!missing.status().is_directory().is_ok());
    assert(!missing.status().is_regular_file().is_ok());
    assert(!missing.status().permissions().is_ok());

#if defined(__unix__) || defined(__APPLE__)
    // A named pipe is neither a file nor a directory, so copying it is refused.
    assert(mkfifo((root + "/pipe").c_str(), 0600) == 0);
    fs::path pipe{(root + "/pipe").c_str()};
    assert(!pipe.copy(first).is_ok());

    // A write that the device refuses reports the failure. The null device of Linux and macOS
    // has no room for data, and the write fails there.
    if (std_fs::exists("/dev/full"))
    {
        assert(!fs::fs_detail::copy_file(first.absolute_name().c_str(), "/dev/full").is_ok());
    }
#endif

    std_fs::remove_all(root);
    return 0;
}
