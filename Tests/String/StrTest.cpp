#include <String.hpp>
#include <Io.hpp>

int main()
{
    // copy and move
    {
        hsd_println("Copy and move:");
        hsd::string str = "Test String";
        auto copy_str = str;
        auto move_str = hsd::move(copy_str);
        copy_str = str;
        move_str = hsd::move(copy_str);
        copy_str = "Other String";
    }

    // printing
    {
        hsd_print("\nPrinting:\n");
        hsd::string str = "Test String";
        hsd_println("{}", str);
    }

    // iterating
    {
        hsd_print("\nIterating:\n");
        hsd::string str = "Test String";

        for (char& ch : str)
            ch++;

        hsd_println("{}", str);
    }

    // find
    {
        hsd_print("\nFinding:\n");
        hsd::string str = "Test String";

        hsd_println("{}", str.find('r'));
        hsd_println("{}", str.find("ring"));
        hsd_println("{}", str.rfind('t'));
        hsd_println("{}", str.rfind("es"));
        hsd_println("{}", str.find('w'));
        hsd_println("{}", str.find("ete"));
        hsd_println("{}", str.rfind('w'));
        hsd_println("{}", str.rfind("ete"));
    }

    // comparison take 1
    {
        hsd_print("\nComparison 1:\n");
        hsd::string str = "Test String";
        hsd::string str2 = "Test String2";

        if (str <= str2)
        {
            if (str == str2)
            {
                hsd_println_err("BUG!");
            }
            if (str < str2)
            {
                hsd_println("OK!");
            }
        }
        if (str >= str2)
        {
            if (str == str2)
            {
                hsd_println_err("BUG!");
            }
            if (str > str2)
            {
                hsd_println_err("BUG!");
            }
        }
        if (str != str2)
        {
            hsd_println("OK!");
        }
    }

    // comparison take 2
    {
        hsd_print("\nComparison 2:\n");
        hsd::string str = "Test String3";
        hsd::string str2 = "Test String2";

        if (str <= str2)
        {
            if (str == str2)
            {
                hsd_println_err("BUG!");
            }
            if (str < str2)
            {
                hsd_println_err("BUG!");
            }
        }
        if (str >= str2)
        {
            if (str == str2)
            {
                hsd_println_err("BUG!");
            }
            if (str > str2)
            {
                hsd_println("OK!");
            }
        }
        if (str != str2)
        {
            hsd_println("OK!");
        }
    }

    // comparison take 3
    {
        hsd_print("\nComparison 3:\n");
        hsd::string str = "Test String2";
        hsd::string str2 = "Test String2";

        if (str <= str2)
        {
            if (str == str2)
            {
                hsd_println("OK!");
            }
            if (str < str2)
            {
                hsd_println_err("BUG!");
            }
        }
        if (str >= str2)
        {
            if (str == str2)
            {
                hsd_println("OK!");
            }
            if (str > str2)
            {
                hsd_println_err("BUG!");
            }
        }
        if (str != str2)
        {
            hsd_println("BUG!");
        }
    }

    // comparison take 4
    {
        hsd_print("\nComparison 4:\n");
        hsd::string str = "Test String34";
        hsd::string str2 = "Test String2";

        if (str <= str2)
        {
            if (str == str2)
            {
                hsd_println_err("BUG!");
            }
            if (str < str2)
            {
                hsd_println_err("BUG!");
            }
        }
        if (str >= str2)
        {
            if (str == str2)
            {
                hsd_println_err("BUG!");
            }
            if (str > str2)
            {
                hsd_println("OK!");
            }
        }
        if (str != str2)
        {
            hsd_println("OK!");
        }
    }

    // erasing
    {
        hsd_print("\nErasing:\n");
        hsd::string str = "Test long String";

        for (auto it = str.begin(); it != str.end();)
        {
            if (*it == 't' || *it == 'n')
            {
                it = str.erase(it).unwrap();
            }
            else
            {
                it++;
            }
        }

        hsd_println("{}", str);
    }

    // push/emplace_back
    {
        hsd_print("\nPushing:\n");
        hsd::string str;

        for (hsd::i32 index = 0; index < 10; index++)
            str.push_back('0' + static_cast<char>(index));

        hsd_println("{}", str);
    }

    // contains, starts/ends_with
    {
        hsd_print("\n\"contains, starts/ends_with\":\n");
        hsd::string str = "Test String";

        hsd_println("Starts with letter:");
        hsd_println("{}", str.starts_with('T'));
        hsd_println("{}", str.starts_with('E'));
        hsd_println("Starts with sequence:");
        hsd_println("{}", str.starts_with("Tes"));
        hsd_println("{}", str.starts_with("Ter"));
        hsd_println("Starts with string:");
        hsd_println("{}", str.starts_with(hsd::string{"Tes"}));
        hsd_println("{}", str.starts_with(hsd::string{"Ter"}));
        hsd_println("Contains letter:");
        hsd_println("{}", str.contains('t'));
        hsd_println("{}", str.contains('q'));
        hsd_println("Contains sequence:");
        hsd_println("{}", str.contains("Str"));
        hsd_println("{}", str.contains("Ste"));
        hsd_println("Contains string:");
        hsd_println("{}", str.contains(hsd::string{"Str"}));
        hsd_println("{}", str.contains(hsd::string{"Ste"}));
        hsd_println("Ends with letter:");
        hsd_println("{}", str.ends_with('g'));
        hsd_println("{}", str.ends_with('e'));
        hsd_println("Ends with sequence:");
        hsd_println("{}", str.ends_with("ing"));
        hsd_println("{}", str.ends_with("ind"));
        hsd_println("Ends with string:");
        hsd_println("{}", str.ends_with(hsd::string{"ing"}));
        hsd_println("{}", str.ends_with(hsd::string{"ind"}));
    }

    // sub_string
    {
        hsd_print("\n\"sub_string\":\n");
        hsd::string str = "Test String";

        hsd_println("{}", str.sub_string(5, 3).unwrap());
        hsd_println("{}", str.sub_string(5).unwrap());
        hsd_println("{}", str.sub_string(4, 8).unwrap_err()());
    }

    return 0;
}