#include <Path.hpp>
#include <Io.hpp>

namespace fs = hsd::filesystem;

static void file_status(const fs::path& p)
{
    auto file_stat = p.status();
    auto file_perms = file_stat.permissions().unwrap();

    hsd_println("Name:           {}", p.relative_name()                   );
    hsd_println("Extension:      {}", p.extension()                       );
    hsd_println("Directory size: {}", file_stat.size()           .unwrap());
    hsd_println("Is Directory:   {}", file_stat.is_directory()   .unwrap());
    hsd_println("Is Symlink:     {}", file_stat.is_symlink()     .unwrap());
    hsd_println("Is Block file:  {}", file_stat.is_block_file()  .unwrap());
    hsd_println("Is Character:   {}", file_stat.is_character()   .unwrap());
    hsd_println("Is FIFO file:   {}", file_stat.is_fifo_file()   .unwrap());
    hsd_println("Is File:        {}", file_stat.is_regular_file().unwrap());
    hsd_println("Is Socket:      {}", file_stat.is_socket()      .unwrap());
    
    puts("\nFile Permissions:");

    hsd_println("Can Owner Read:             {}", file_perms.can_owner_read          );
    hsd_println("Can Owner Write:            {}", file_perms.can_owner_write         );
    hsd_println("Can Owner Execute:          {}", file_perms.can_owner_exec          );
    hsd_println("Can Owner Do everything:    {}", file_perms.can_owner_do_everything );
    hsd_println("Can Group Read:             {}", file_perms.can_group_read          );
    hsd_println("Can Group Write:            {}", file_perms.can_group_write         );
    hsd_println("Can Group Execute:          {}", file_perms.can_group_exec          );
    hsd_println("Can Group Do everything:    {}", file_perms.can_group_do_everything );
    hsd_println("Can Others Read:            {}", file_perms.can_others_read         );
    hsd_println("Can Others Write:           {}", file_perms.can_others_write        );
    hsd_println("Can Others Execute:         {}", file_perms.can_others_exec         );
    hsd_println("Can Others Do everything:   {}", file_perms.can_others_do_everything);
    hsd_println("Can Everyone Do everything: {}", file_perms.can_all_do_everything   );
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
        hsd_print(" \"{}\" |", parent);
    }

    //fs::path p2{"testsrc"};
    //p2.copy("testcopy").unwrap();
    puts("");
    return 0;
}