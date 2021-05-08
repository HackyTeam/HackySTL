#include <String.hpp>
#include <Io.hpp>

int main()
{
    // copy and move
    {
        hsd::io::print<"Copy and move:\n">();
        hsd::string str = "Test String";
        auto copy_str = str;
        auto move_str = hsd::move(copy_str);
        copy_str = str;
        move_str = hsd::move(copy_str);
        copy_str = "Other String";
    }

    // printing
    {
        hsd::io::print<"\nPrinting:\n">();
        hsd::string str = "Test String";
        hsd::io::print<"{}\n">(str);
    }

    // iterating
    {
        hsd::io::print<"\nIterating:\n">();
        hsd::string str = "Test String";

        for(char& ch : str)
            ch++;

        hsd::io::print<"{}\n">(str);
    }

    // find
    {
        hsd::io::print<"\nFinding:\n">();
        hsd::string str = "Test String";

        hsd::io::print<"{}\n">(str.find('r'));
        hsd::io::print<"{}\n">(str.find("ring"));
        hsd::io::print<"{}\n">(str.rfind('t'));
        hsd::io::print<"{}\n">(str.rfind("es"));
        hsd::io::print<"{}\n">(str.find('w'));
        hsd::io::print<"{}\n">(str.find("ete"));
        hsd::io::print<"{}\n">(str.rfind('w'));
        hsd::io::print<"{}\n">(str.rfind("ete"));
    }

    // comparison take 1
    {
        hsd::io::print<"\nComparison 1:\n">();
        hsd::string str = "Test String";
        hsd::string str2 = "Test String2";

        if(str <= str2)
        {
            if(str == str2)
            {
                hsd::io::err_print<"BUG!\n">();
            }
            if(str < str2)
            {
                hsd::io::print<"OK!\n">();
            }
        }
        if(str >= str2)
        {
            if(str == str2)
            {
                hsd::io::err_print<"BUG!\n">();
            }
            if(str > str2)
            {
                hsd::io::err_print<"BUG!\n">();
            }
        }
        if(str != str2)
        {
            hsd::io::print<"OK!\n">();
        }
    }

    // comparison take 2
    {
        hsd::io::print<"\nComparison 2:\n">();
        hsd::string str = "Test String3";
        hsd::string str2 = "Test String2";

        if(str <= str2)
        {
            if(str == str2)
            {
                hsd::io::err_print<"BUG!\n">();
            }
            if(str < str2)
            {
                hsd::io::print<"BUG!\n">();
            }
        }
        if(str >= str2)
        {
            if(str == str2)
            {
                hsd::io::err_print<"BUG!\n">();
            }
            if(str > str2)
            {
                hsd::io::err_print<"OK!\n">();
            }
        }
        if(str != str2)
        {
            hsd::io::print<"OK!\n">();
        }
    }

    // comparison take 3
    {
        hsd::io::print<"\nComparison 3:\n">();
        hsd::string str = "Test String2";
        hsd::string str2 = "Test String2";

        if(str <= str2)
        {
            if(str == str2)
            {
                hsd::io::err_print<"OK!\n">();
            }
            if(str < str2)
            {
                hsd::io::print<"BUG!\n">();
            }
        }
        if(str >= str2)
        {
            if(str == str2)
            {
                hsd::io::err_print<"OK!\n">();
            }
            if(str > str2)
            {
                hsd::io::err_print<"BUG!\n">();
            }
        }
        if(str != str2)
        {
            hsd::io::print<"BUG!\n">();
        }
    }

    // comparison take 4
    {
        hsd::io::print<"\nComparison 4:\n">();
        hsd::string str = "Test String34";
        hsd::string str2 = "Test String2";

        if(str <= str2)
        {
            if(str == str2)
            {
                hsd::io::err_print<"BUG!\n">();
            }
            if(str < str2)
            {
                hsd::io::print<"BUG!\n">();
            }
        }
        if(str >= str2)
        {
            if(str == str2)
            {
                hsd::io::err_print<"BUG!\n">();
            }
            if(str > str2)
            {
                hsd::io::err_print<"OK!\n">();
            }
        }
        if(str != str2)
        {
            hsd::io::print<"OK!\n">();
        }
    }

    // erasing
    {
        hsd::io::print<"\nErasing:\n">();
        hsd::string str = "Test long String";

        for(auto it = str.begin(); it != str.end();)
        {
            if(*it == 't' || *it == 'n')
            {
                it = str.erase(it).unwrap();
            }
            else
            {
                it++;
            }
        }

        hsd::io::print<"{}\n">(str);
    }

    // push/emplace_back
    {
        hsd::io::print<"\nPushing:\n">();
        hsd::string str;

        for(hsd::i32 index = 0; index < 10; index++)
            str.push_back('0' + static_cast<char>(index));

        hsd::io::print<"{}\n">(str);
    }

    // contains, starts/ends_with
    {
        hsd::io::print<"\n\"contains, starts/ends_with\":\n">();
        hsd::string str = "Test String";

        hsd::io::print<"Starts with letter:\n">();
        hsd::io::print<"{}\n">(str.starts_with('T'));
        hsd::io::print<"{}\n">(str.starts_with('E'));
        hsd::io::print<"Starts with sequence:\n">();
        hsd::io::print<"{}\n">(str.starts_with("Tes"));
        hsd::io::print<"{}\n">(str.starts_with("Ter"));
        hsd::io::print<"Starts with string:\n">();
        hsd::io::print<"{}\n">(str.starts_with(hsd::string{"Tes"}));
        hsd::io::print<"{}\n">(str.starts_with(hsd::string{"Ter"}));
        hsd::io::print<"Contains letter:\n">();
        hsd::io::print<"{}\n">(str.contains('t'));
        hsd::io::print<"{}\n">(str.contains('q'));
        hsd::io::print<"Contains sequence:\n">();
        hsd::io::print<"{}\n">(str.contains("Str"));
        hsd::io::print<"{}\n">(str.contains("Ste"));
        hsd::io::print<"Contains string:\n">();
        hsd::io::print<"{}\n">(str.contains(hsd::string{"Str"}));
        hsd::io::print<"{}\n">(str.contains(hsd::string{"Ste"}));
        hsd::io::print<"Ends with letter:\n">();
        hsd::io::print<"{}\n">(str.ends_with('g'));
        hsd::io::print<"{}\n">(str.ends_with('e'));
        hsd::io::print<"Ends with sequence:\n">();
        hsd::io::print<"{}\n">(str.ends_with("ing"));
        hsd::io::print<"{}\n">(str.ends_with("ind"));
        hsd::io::print<"Ends with string:\n">();
        hsd::io::print<"{}\n">(str.ends_with(hsd::string{"ing"}));
        hsd::io::print<"{}\n">(str.ends_with(hsd::string{"ind"}));
    }

    // sub_string
    {
        hsd::io::print<"\n\"sub_string\":\n">();
        hsd::string str = "Test String";

        hsd::io::print<"{}\n">(str.sub_string(5, 3).unwrap());
        hsd::io::print<"{}\n">(str.sub_string(5).unwrap());
        hsd::io::print<"{}\n">(str.sub_string(4, 8).unwrap_err()());
    }

    return 0;
}