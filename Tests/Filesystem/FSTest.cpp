#include <Path.hpp>
#include <Io.hpp>

namespace fs = hsd::filesystem;

static const char* to_bool_str(bool b)
{
    return b ? "true" : "false";
}

static void file_status(const fs::path& p)
{
    auto file_stat = p.status();
    auto file_perms = file_stat.permissions().unwrap();

    hsd_println("Name:           {}", p.relative_name()                                );
    hsd_println("Extension:      {}", p.extension()                                    );
    hsd_println("Directory size: {}", to_bool_str(file_stat.size().unwrap())           );
    hsd_println("Is Directory:   {}", to_bool_str(file_stat.is_directory().unwrap())   );
    hsd_println("Is Symlink:     {}", to_bool_str(file_stat.is_symlink().unwrap())     );
    hsd_println("Is Block file:  {}", to_bool_str(file_stat.is_block_file().unwrap())  );
    hsd_println("Is Character:   {}", to_bool_str(file_stat.is_character().unwrap())   );
    hsd_println("Is FIFO file:   {}", to_bool_str(file_stat.is_fifo_file().unwrap())   );  
    hsd_println("Is File:        {}", to_bool_str(file_stat.is_regular_file().unwrap()));
    hsd_println("Is Socket:      {}", to_bool_str(file_stat.is_socket().unwrap())      );
    
    puts("\nFile Permissions:");

    hsd_println("Can Owner Read:             {}", to_bool_str(file_perms.can_owner_read)          );
    hsd_println("Can Owner Write:            {}", to_bool_str(file_perms.can_owner_write)         );
    hsd_println("Can Owner Execute:          {}", to_bool_str(file_perms.can_owner_exec)          );
    hsd_println("Can Owner Do everything:    {}", to_bool_str(file_perms.can_owner_do_everything) );
    hsd_println("Can Group Read:             {}", to_bool_str(file_perms.can_group_read)          );
    hsd_println("Can Group Write:            {}", to_bool_str(file_perms.can_group_write)         );
    hsd_println("Can Group Execute:          {}", to_bool_str(file_perms.can_group_exec)          );
    hsd_println("Can Group Do everything:    {}", to_bool_str(file_perms.can_group_do_everything) );
    hsd_println("Can Others Read:            {}", to_bool_str(file_perms.can_others_read)         );
    hsd_println("Can Others Write:           {}", to_bool_str(file_perms.can_others_write)        );
    hsd_println("Can Others Execute:         {}", to_bool_str(file_perms.can_others_exec)         );
    hsd_println("Can Others Do everything:   {}", to_bool_str(file_perms.can_others_do_everything));
    hsd_println("Can Everyone Do everything: {}", to_bool_str(file_perms.can_all_do_everything)   );
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