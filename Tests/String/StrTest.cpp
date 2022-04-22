#include <String.hpp>
#include <Io.hpp>

int main()
{
    using namespace hsd::format_literals;

    // copy and move
    {
        hsd::println("Copy and move:"_fmt);
        hsd::string str = "Test String";
        auto copy_str = str;
        auto move_str = hsd::move(copy_str);
        copy_str = str;
        move_str = hsd::move(copy_str);
        copy_str = "Other String";
    }

    // printing
    {
        hsd::print("\nPrinting:\n"_fmt);
        hsd::string str = "Test String";
        hsd::println("{}"_fmt, str);
    }

    // iterating
    {
        hsd::print("\nIterating:\n"_fmt);
        hsd::string str = "Test String";

        for (char& ch : str)
            ch++;

        hsd::println("{}"_fmt, str);
    }

    // find
    {
        hsd::print("\nFinding:\n"_fmt);
        hsd::string str = "Test String";

        hsd::println("{}"_fmt, str.find('r'));
        hsd::println("{}"_fmt, str.find("ring"));
        hsd::println("{}"_fmt, str.rfind('t'));
        hsd::println("{}"_fmt, str.rfind("es"));
        hsd::println("{}"_fmt, str.find('w'));
        hsd::println("{}"_fmt, str.find("ete"));
        hsd::println("{}"_fmt, str.rfind('w'));
        hsd::println("{}"_fmt, str.rfind("ete"));
    }

    // comparison take 1
    {
        hsd::print("\nComparison 1:\n"_fmt);
        hsd::string str = "Test String";
        hsd::string str2 = "Test String2";

        if (str <= str2)
        {
            if (str == str2)
            {
                hsd::println_err("BUG!"_fmt);
            }
            if (str < str2)
            {
                hsd::println("OK!"_fmt);
            }
        }
        if (str >= str2)
        {
            if (str == str2)
            {
                hsd::println_err("BUG!"_fmt);
            }
            if (str > str2)
            {
                hsd::println_err("BUG!"_fmt);
            }
        }
        if (str != str2)
        {
            hsd::println("OK!"_fmt);
        }
    }

    // comparison take 2
    {
        hsd::print("\nComparison 2:\n"_fmt);
        hsd::string str = "Test String3";
        hsd::string str2 = "Test String2";

        if (str <= str2)
        {
            if (str == str2)
            {
                hsd::println_err("BUG!"_fmt);
            }
            if (str < str2)
            {
                hsd::println_err("BUG!"_fmt);
            }
        }
        if (str >= str2)
        {
            if (str == str2)
            {
                hsd::println_err("BUG!"_fmt);
            }
            if (str > str2)
            {
                hsd::println("OK!"_fmt);
            }
        }
        if (str != str2)
        {
            hsd::println("OK!"_fmt);
        }
    }

    // comparison take 3
    {
        hsd::print("\nComparison 3:\n"_fmt);
        hsd::string str = "Test String2";
        hsd::string str2 = "Test String2";

        if (str <= str2)
        {
            if (str == str2)
            {
                hsd::println("OK!"_fmt);
            }
            if (str < str2)
            {
                hsd::println_err("BUG!"_fmt);
            }
        }
        if (str >= str2)
        {
            if (str == str2)
            {
                hsd::println("OK!"_fmt);
            }
            if (str > str2)
            {
                hsd::println_err("BUG!"_fmt);
            }
        }
        if (str != str2)
        {
            hsd::println("BUG!"_fmt);
        }
    }

    // comparison take 4
    {
        hsd::print("\nComparison 4:\n"_fmt);
        hsd::string str = "Test String34";
        hsd::string str2 = "Test String2";

        if (str <= str2)
        {
            if (str == str2)
            {
                hsd::println_err("BUG!"_fmt);
            }
            if (str < str2)
            {
                hsd::println_err("BUG!"_fmt);
            }
        }
        if (str >= str2)
        {
            if (str == str2)
            {
                hsd::println_err("BUG!"_fmt);
            }
            if (str > str2)
            {
                hsd::println("OK!"_fmt);
            }
        }
        if (str != str2)
        {
            hsd::println("OK!"_fmt);
        }
    }

    // erasing
    {
        hsd::print("\nErasing:\n"_fmt);
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

        hsd::println("{}"_fmt, str);
    }

    // push/emplace_back
    {
        hsd::print("\nPushing:\n"_fmt);
        hsd::string str;

        for (hsd::i32 index = 0; index < 10; index++)
            str.push_back('0' + static_cast<char>(index));

        hsd::println("{}"_fmt, str);
    }

    // contains, starts/ends_with
    {
        hsd::print("\n\"contains, starts/ends_with\":\n"_fmt);
        hsd::string str = "Test String";

        hsd::println("Starts with letter:"_fmt);
        hsd::println("{}"_fmt, str.starts_with('T') ? "true" : "false");
        hsd::println("{}"_fmt, str.starts_with('E') ? "true" : "false");
        hsd::println("Starts with sequence:"_fmt);
        hsd::println("{}"_fmt, str.starts_with("Tes") ? "true" : "false");
        hsd::println("{}"_fmt, str.starts_with("Ter") ? "true" : "false");
        hsd::println("Starts with string:"_fmt);
        hsd::println("{}"_fmt, str.starts_with(hsd::string{"Tes"}) ? "true" : "false");
        hsd::println("{}"_fmt, str.starts_with(hsd::string{"Ter"}) ? "true" : "false");
        hsd::println("Contains letter:"_fmt);
        hsd::println("{}"_fmt, str.contains('t') ? "true" : "false");
        hsd::println("{}"_fmt, str.contains('q') ? "true" : "false");
        hsd::println("Contains sequence:"_fmt);
        hsd::println("{}"_fmt, str.contains("Str") ? "true" : "false");
        hsd::println("{}"_fmt, str.contains("Ste") ? "true" : "false");
        hsd::println("Contains string:"_fmt);
        hsd::println("{}"_fmt, str.contains(hsd::string{"Str"}) ? "true" : "false");
        hsd::println("{}"_fmt, str.contains(hsd::string{"Ste"}) ? "true" : "false");
        hsd::println("Ends with letter:"_fmt);
        hsd::println("{}"_fmt, str.ends_with('g') ? "true" : "false");
        hsd::println("{}"_fmt, str.ends_with('e') ? "true" : "false");
        hsd::println("Ends with sequence:"_fmt);
        hsd::println("{}"_fmt, str.ends_with("ing") ? "true" : "false");
        hsd::println("{}"_fmt, str.ends_with("ind") ? "true" : "false");
        hsd::println("Ends with string:"_fmt);
        hsd::println("{}"_fmt, str.ends_with(hsd::string{"ing"}) ? "true" : "false");
        hsd::println("{}"_fmt, str.ends_with(hsd::string{"ind"}) ? "true" : "false");
    }

    // sub_string
    {
        hsd::print("\n\"sub_string\":\n"_fmt);
        hsd::string str = "Test String";

        hsd::println("{}"_fmt, str.sub_string(5, 3).unwrap());
        hsd::println("{}"_fmt, str.sub_string(5).unwrap());
        hsd::println("{}"_fmt, str.sub_string(4, 8).unwrap_err()());
    }

    // String View bonus
    {
        hsd::print("\nString View bonus:\n"_fmt);
        hsd::string str = "Test String";
        hsd::radix_sort(str, [](char v1, char v2) { return v1 > v2; });

        hsd::println("{fg=214,bg=73}"_fmt, hsd::string_view{str.c_str(), 4});
    }

    return 0;
}