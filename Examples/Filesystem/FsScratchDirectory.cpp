// Title: A scratch directory
// Level: Intermediate
// Shows: creating a directory and files, listing it in sorted order, copying a file, and reading the copy back.

#include "../Support/ExampleSupport.hpp"
#include <Containers/String.hpp>
#include <Containers/Vector.hpp>
#include <Base/Algorithm.hpp>
#include <Filesystem/Path.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;
namespace fs = hsd::filesystem;

static bool write_text(const char* name, const char* text)
{
    auto opened = hsd::io::load_file(name, hsd::io_options::write);

    if (!opened.is_ok())
        return false;

    {
        auto file = opened.unwrap();
        file.print("{}"_fmt, text).flush().unwrap();
    }

    // Created files start with platform-default permissions; set them explicitly.
    fs::path created{name};
    return created.change_permissions(
        fs::fs_perms_mask::owner_read | fs::fs_perms_mask::owner_write).is_ok();
}

// Lists a directory. list() returns the entries in whatever order the system
// gives them, so the names are sorted before they are shown.
static void show_directory(const char* label, fs::path& directory)
{
    auto listed = directory.list();

    if (!listed.is_ok())
    {
        hsd::println("{}: cannot list"_fmt, label);
        return;
    }

    auto entries = listed.unwrap();
    hsd::vector<hsd::string> names;

    for (auto& entry : entries)
    {
        names.push_back(example_support::with_forward_slashes(entry.relative_name()));
    }

    // The default predicate sorts ascending.
    hsd::quick_sort(names);

    hsd::println("{} ({} entries)"_fmt, label, names.size());

    for (auto& name : names)
    {
        fs::path entry{name.c_str()};
        hsd::println("  {} {} bytes"_fmt, name, entry.status().size().unwrap());
    }
}

int main()
{
    fs::path here{"."};

    // create_directory() makes one level and does not complain if it exists,
    // so running the program twice in the same place is fine.
    here.create_directory("scratch").unwrap();
    here.create_directory("backup").unwrap();
    here.create_directory("scratch").unwrap();

    write_text("./scratch/todo.txt", "buy milk\nwrite examples\n");
    write_text("./scratch/data.csv", "id,value\n1,10\n2,20\n3,30\n");
    write_text("./scratch/empty.log", "");

    fs::path scratch{"./scratch"};
    fs::path backup{"./backup"};
    show_directory("scratch", scratch);
    show_directory("backup", backup);

    // Copying a file into a directory keeps its name.
    fs::path todo{"./scratch/todo.txt"};
    auto copied = todo.copy(backup);
    hsd::println("copy succeeded: {}"_fmt, copied.is_ok() ? "true" : "false");

    // A path that has already listed its directory can report stale
    // entries afterwards, so the directory is listed through a fresh path.
    fs::path backup_after_copy{"./backup"};
    show_directory("backup", backup_after_copy);

    // Read the copy back through the same io type used for any other file.
    auto opened = hsd::io::load_file("./backup/todo.txt", hsd::io_options::read);

    if (opened.is_ok())
    {
        auto file = opened.unwrap();
        auto chunk = file.read_chunk();

        if (chunk.is_ok())
        {
            hsd::println("copy holds {} characters"_fmt, chunk.unwrap().get().get_stream_data().length());
        }
    }

    // Asking about files that were never created is safe.
    fs::path never{"./backup/data.csv"};
    hsd::println("data.csv in backup: {}"_fmt, never.status().exists() ? "yes" : "no");

    // rename() keeps the file in its directory. Listing the directory afterwards
    // shows the new name through the same path object.
    fs::path copied_file{"./backup/todo.txt"};
    hsd::println("renamed: {}"_fmt, copied_file.rename("notes.txt").is_ok() ? "true" : "false");
    show_directory("backup after rename", backup);
    return 0;
}
