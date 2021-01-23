#include <Path.hpp>
#include <Io.hpp>

namespace fs = hsd::filesystem;

static void file_status(const fs::path& p)
{
    hsd::io::print<"Name: {}\n">(p.relative_name());
    hsd::io::print<"Directory size: {}b\n">(p.status().size().unwrap());
    hsd::io::print<"Is Directory: {}\n">(p.status().is_directory().unwrap());
    hsd::io::print<"Is Symlink: {}\n">(p.status().is_symlink().unwrap());
    hsd::io::print<"Is Block file: {}\n">(p.status().is_block_file().unwrap());
    hsd::io::print<"Is Character: {}\n">(p.status().is_character().unwrap());
    hsd::io::print<"Is FIFO file: {}\n">(p.status().is_fifo_file().unwrap());
    hsd::io::print<"Is File: {}\n">(p.status().is_regular_file().unwrap());
    hsd::io::print<"Is Socket: {}\n">(p.status().is_socket().unwrap());
}

int main()
{
    fs::path p{".vscode"};

    file_status(p);
    puts("");
    file_status(p / "..");
    puts("");
    file_status(p / ".." / "..");
    puts("");

    for(const auto& path : p.list().unwrap())
    {
        file_status(path);
        puts("");
    }

    for(auto& parent : p)
    {
        hsd::io::print<" \"{}\" |">(parent);
    }

    puts("");
    return 0;
}