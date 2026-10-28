// Title: Compile-time number text
// Level: Advanced
// Shows: to_pseudo_string in constant expressions, building a text table at compile time.

#include <Containers/StringView.hpp>
#include <Serialize/Io.hpp>
#include <Serialize/ToString.hpp>

using namespace hsd::format_literals;

namespace
{
    // A plain fixed buffer that can be built and checked during compilation.
    template <hsd::usize N>
    struct text_table
    {
        char data[N] = {};
        hsd::usize length = 0;

        constexpr void append(char ch)
        {
            data[length++] = ch;
        }

        // Copies the characters of any pseudo string into the table.
        constexpr void append(const auto& pseudo)
        {
            for (hsd::usize i = 0; i < pseudo.size(); ++i)
            {
                append(pseudo.data()[i]);
            }
        }

        constexpr bool equals(const char* expected) const
        {
            hsd::usize i = 0;

            for (; expected[i] != '\0'; ++i)
            {
                if (i >= length || data[i] != expected[i])
                {
                    return false;
                }
            }

            return i == length;
        }
    };

    // Writes "n:n*n" pairs, one per entry, separated by commas.
    constexpr auto make_squares()
    {
        text_table<64> table;

        for (hsd::i32 n = 1; n <= 6; ++n)
        {
            if (n != 1)
            {
                table.append(',');
            }

            table.append(hsd::to_pseudo_string(n));
            table.append(':');
            table.append(hsd::to_pseudo_string(n * n));
        }

        return table;
    }

    // Mixes decimal, hexadecimal and float text in one constant.
    constexpr auto make_report()
    {
        text_table<96> table;

        table.append(hsd::to_pseudo_string(-2024));
        table.append(' ');
        table.append(hsd::to_pseudo_string_hex(48879));
        table.append(' ');
        table.append(hsd::to_pseudo_string<char, 3>(2.0 / 3.0));
        table.append(' ');
        table.append(hsd::to_pseudo_string_exp<char, 1>(1500.0));

        return table;
    }

    constexpr auto squares = make_squares();
    constexpr auto report = make_report();

    // These checks run while compiling, so a wrong table would not build.
    static_assert(squares.equals("1:1,2:4,3:9,4:16,5:25,6:36"));
    static_assert(report.equals("-2024 0xbeef 0.667 1.5e+03"));
    static_assert(squares.length == 26);
}

int main()
{
    hsd::string_view squares_view{squares.data, squares.length};
    hsd::string_view report_view{report.data, report.length};

    hsd::println("squares: {}"_fmt, squares_view);
    hsd::println("length: {}"_fmt, squares.length);
    hsd::println("report: {}"_fmt, report_view);

    // The same call at run time gives the same text.
    hsd::i32 runtime_value = static_cast<hsd::i32>(squares.length) * 78;
    auto runtime_text = hsd::to_pseudo_string(runtime_value);
    hsd::string_view runtime_view{runtime_text.data(), runtime_text.size()};
    hsd::println("runtime value: {}"_fmt, runtime_view);

    return 0;
}
