// Title: Inspecting files and directories
// Level: Basic
// Shows: path::status(), exists/is_directory/is_regular_file/size/permissions, extension(), and checking a missing path.

#include "../Support/ExampleSupport.hpp"
#include <Filesystem/Path.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;
namespace fs = hsd::filesystem;

static const char* yes_no(bool value)
{
    return value ? "true" : "false";
}

// Creates a small text file. Everything happens in the run directory.
static bool write_text(const char* name, const char* text)
{
    auto opened = hsd::io::load_file(name, hsd::io_options::write);

    if (!opened.is_ok())
        return false;

    {
        auto file = opened.unwrap();
        file.print("{}"_fmt, text).flush().unwrap();
    }

    // A freshly created file gets whatever permissions the platform defaults to,
    // so state the ones we want: read and write for the owner.
    fs::path created{name};
    return created.change_permissions(
        fs::fs_perms_mask::owner_read | fs::fs_perms_mask::owner_write).is_ok();
}

int main()
{
    fs::path here{"."};
    here.create_directory("inspect_demo").unwrap();

    if (!write_text("./inspect_demo/notes.txt", "first line\nsecond line\n"))
    {
        hsd::println("could not create the demo file"_fmt);
        return 0;
    }

    // A path is a name plus a view of what is on disk; status() asks the system.
    fs::path folder{"./inspect_demo"};
    auto folder_status = folder.status();
    hsd::println("folder: {}"_fmt, example_support::with_forward_slashes(folder.relative_name()));
    hsd::println("exists: {}"_fmt, yes_no(folder_status.exists()));
    hsd::println("is directory: {}"_fmt, yes_no(folder_status.is_directory().unwrap()));
    hsd::println("is regular file: {}"_fmt, yes_no(folder_status.is_regular_file().unwrap()));

    fs::path notes{"./inspect_demo/notes.txt"};
    auto notes_status = notes.status();
    hsd::println("file: {}"_fmt, example_support::with_forward_slashes(notes.relative_name()));
    hsd::println("is regular file: {}"_fmt, yes_no(notes_status.is_regular_file().unwrap()));
    hsd::println("size in bytes: {}"_fmt, notes_status.size().unwrap());

    // extension() includes the dot, and it is only meaningful for an existing file.
    hsd::println("extension: {}"_fmt, notes.extension().c_str());

    auto permissions = notes_status.permissions().unwrap();
    hsd::println("owner can read: {}"_fmt, yes_no(permissions.can_owner_read));
    hsd::println("owner can write: {}"_fmt, yes_no(permissions.can_owner_write));
    hsd::println("owner can execute: {}"_fmt, yes_no(permissions.can_owner_exec));

    // A missing path is not an error by itself: status().exists() is false, and
    // the queries that need a real file return a failed result instead of aborting.
    fs::path missing{"./inspect_demo/missing.md"};
    auto missing_status = missing.status();
    hsd::println("missing exists: {}"_fmt, yes_no(missing_status.exists()));
    hsd::println("missing size known: {}"_fmt, yes_no(missing_status.size().is_ok()));

    return 0;
}
