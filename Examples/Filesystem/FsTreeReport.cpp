// Title: Directory tree report
// Level: Advanced
// Shows: building a small project tree, walking it recursively with sorted list(), per-directory totals, and an extension tally.

#include <Containers/String.hpp>
#include <Containers/Vector.hpp>
#include <Base/Algorithm.hpp>
#include <Filesystem/Path.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;
namespace fs = hsd::filesystem;

// Writes `size` bytes of filler into a new file, then fixes its permissions.
static bool write_file(const char* name, hsd::usize size)
{
    auto opened = hsd::io::load_file(name, hsd::io_options::write);

    if (!opened.is_ok())
        return false;

    {
        auto file = opened.unwrap();
        hsd::string text;

        for (hsd::usize i = 0; i < size; i++)
        {
            text.push_back(static_cast<char>('a' + i % 26));
        }

        file.print("{}"_fmt, text).flush().unwrap();
    }

    fs::path created{name};
    return created.change_permissions(
        fs::fs_perms_mask::owner_read | fs::fs_perms_mask::owner_write).is_ok();
}

struct extension_count
{
    hsd::string extension;
    hsd::usize files = 0;
    hsd::usize bytes = 0;

    // quick_sort only needs '>' to order the elements.
    bool operator>(const extension_count& rhs) const
    {
        return extension > rhs.extension;
    }
};

struct totals
{
    hsd::usize files = 0;
    hsd::usize directories = 0;
    hsd::usize bytes = 0;
};

// Windows paths use '\' between their parts, and some use '/' too, so both count.
static hsd::string_view base_name(const hsd::string& full)
{
    hsd::usize _start = 0;

    for (hsd::usize _idx = 0; _idx < full.length(); ++_idx)
    {
        if (full[_idx] == '/' || full[_idx] == '\\')
        {
            _start = _idx + 1;
        }
    }

    return hsd::string_view{full.c_str() + _start};
}

static void tally(hsd::vector<extension_count>& counts, const hsd::string& extension, hsd::usize bytes)
{
    // Files without an extension are grouped under "(none)".
    hsd::string key = extension.length() == 0 ? hsd::string{"(none)"} : extension;

    for (auto& item : counts)
    {
        if (item.extension == key)
        {
            item.files++;
            item.bytes += bytes;
            return;
        }
    }

    counts.push_back(extension_count{key, 1, bytes});
}

static void indent(hsd::usize depth)
{
    for (hsd::usize i = 0; i < depth; i++)
    {
        hsd::print("  "_fmt);
    }
}

// Lists a directory in name order. list() gives no particular order, but path
// has operator<, so the default sort puts the entries in name order.
static hsd::vector<fs::path> sorted_entries(fs::path& directory)
{
    auto listed = directory.list();

    if (!listed.is_ok())
        return {};

    auto entries = listed.unwrap();
    hsd::quick_sort(entries);
    return entries;
}

static bool is_directory(fs::path& entry)
{
    return entry.status().is_directory().unwrap_or(false);
}

// Counts files, directories and bytes in everything below `directory`.
static totals measure(fs::path& directory)
{
    totals result;

    for (auto& entry : sorted_entries(directory))
    {
        if (is_directory(entry))
        {
            auto below = measure(entry);
            result.files += below.files;
            result.directories += below.directories + 1;
            result.bytes += below.bytes;
        }
        else
        {
            result.files++;
            result.bytes += entry.status().size().unwrap_or(hsd::usize{0});
        }
    }

    return result;
}

// Prints the tree with a summary after every directory name.
static void print_tree(fs::path& directory, hsd::usize depth)
{
    for (auto& entry : sorted_entries(directory))
    {
        indent(depth);

        if (is_directory(entry))
        {
            auto below = measure(entry);
            hsd::println("{}/ ({} files, {} bytes)"_fmt,
                base_name(entry.relative_name()), below.files, below.bytes);
            print_tree(entry, depth + 1);
        }
        else
        {
            hsd::println("{} {}"_fmt,
                base_name(entry.relative_name()), entry.status().size().unwrap_or(hsd::usize{0}));
        }
    }
}

// Groups every file below `directory` by extension.
static void collect(fs::path& directory, hsd::vector<extension_count>& counts)
{
    for (auto& entry : sorted_entries(directory))
    {
        if (is_directory(entry))
            collect(entry, counts);
        else
            tally(counts, entry.extension(), entry.status().size().unwrap_or(hsd::usize{0}));
    }
}

int main()
{
    fs::path here{"."};
    here.create_directory("project").unwrap();

    fs::path project_root{"./project"};
    project_root.create_directory("src").unwrap();
    project_root.create_directory("docs").unwrap();
    project_root.create_directory("empty").unwrap();

    fs::path source{"./project/src"};
    source.create_directory("detail").unwrap();

    write_file("./project/README.md", 120);
    write_file("./project/Makefile", 40);
    write_file("./project/src/main.cpp", 300);
    write_file("./project/src/util.cpp", 180);
    write_file("./project/src/util.hpp", 60);
    write_file("./project/src/detail/impl.cpp", 500);
    write_file("./project/docs/guide.md", 250);
    write_file("./project/docs/notes.txt", 75);

    // Listing uses a path opened after the tree exists. A path opened before
    // the entries were created can report them as missing.
    fs::path project{"./project"};
    hsd::println("project/"_fmt);
    print_tree(project, 1);

    auto all = measure(project);
    hsd::println("total: {} files, {} directories, {} bytes"_fmt, all.files, all.directories, all.bytes);

    hsd::vector<extension_count> counts;
    collect(project, counts);

    // Order the tally by extension text, then print it.
    hsd::quick_sort(counts);

    hsd::println("by extension:"_fmt);

    for (auto& item : counts)
    {
        hsd::println("  {} {} files {} bytes"_fmt, item.extension, item.files, item.bytes);
    }

    return 0;
}
