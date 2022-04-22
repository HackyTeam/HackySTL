#include <Path.hpp>
#include <Io.hpp>

namespace fs = hsd::filesystem;
using namespace hsd::format_literals;

static const char* to_bool_str(bool b)
{
    return b ? "true" : "false";
}

static void file_status(const fs::path& p)
{
    auto file_stat = p.status();
    auto file_perms = file_stat.permissions().unwrap();

    hsd::println("Name:           {}"_fmt, p.relative_name()                                );
    hsd::println("Extension:      {}"_fmt, p.extension()                                    );
    hsd::println("Directory size: {}"_fmt, to_bool_str(file_stat.size().unwrap())           );
    hsd::println("Is Directory:   {}"_fmt, to_bool_str(file_stat.is_directory().unwrap())   );
    hsd::println("Is Symlink:     {}"_fmt, to_bool_str(file_stat.is_symlink().unwrap())     );
    hsd::println("Is Block file:  {}"_fmt, to_bool_str(file_stat.is_block_file().unwrap())  );
    hsd::println("Is Character:   {}"_fmt, to_bool_str(file_stat.is_character().unwrap())   );
    hsd::println("Is FIFO file:   {}"_fmt, to_bool_str(file_stat.is_fifo_file().unwrap())   );  
    hsd::println("Is File:        {}"_fmt, to_bool_str(file_stat.is_regular_file().unwrap()));
    hsd::println("Is Socket:      {}"_fmt, to_bool_str(file_stat.is_socket().unwrap())      );
    
    puts("\nFile Permissions:");

    hsd::println("Can Owner Read:             {}"_fmt, to_bool_str(file_perms.can_owner_read)          );
    hsd::println("Can Owner Write:            {}"_fmt, to_bool_str(file_perms.can_owner_write)         );
    hsd::println("Can Owner Execute:          {}"_fmt, to_bool_str(file_perms.can_owner_exec)          );
    hsd::println("Can Owner Do everything:    {}"_fmt, to_bool_str(file_perms.can_owner_do_everything) );
    hsd::println("Can Group Read:             {}"_fmt, to_bool_str(file_perms.can_group_read)          );
    hsd::println("Can Group Write:            {}"_fmt, to_bool_str(file_perms.can_group_write)         );
    hsd::println("Can Group Execute:          {}"_fmt, to_bool_str(file_perms.can_group_exec)          );
    hsd::println("Can Group Do everything:    {}"_fmt, to_bool_str(file_perms.can_group_do_everything) );
    hsd::println("Can Others Read:            {}"_fmt, to_bool_str(file_perms.can_others_read)         );
    hsd::println("Can Others Write:           {}"_fmt, to_bool_str(file_perms.can_others_write)        );
    hsd::println("Can Others Execute:         {}"_fmt, to_bool_str(file_perms.can_others_exec)         );
    hsd::println("Can Others Do everything:   {}"_fmt, to_bool_str(file_perms.can_others_do_everything));
    hsd::println("Can Everyone Do everything: {}"_fmt, to_bool_str(file_perms.can_all_do_everything)   );
}

int main()
{
    fs::path p{"."};

    file_status(p);
    puts("");
    file_status(p / "..");
    puts("");
    file_status(p / ".." / "..");
    puts("");

    for (const auto& path : p.list().unwrap())
    {
        file_status(path);
        puts("");
    }

    for (auto& parent : p.parents())
    {
        hsd::print(" \"{}\" |"_fmt, parent);
    }

    //fs::path p2{"testsrc"};
    //p2.copy("testcopy").unwrap();
    puts("");
    return 0;
}