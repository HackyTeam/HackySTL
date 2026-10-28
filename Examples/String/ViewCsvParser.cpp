// Title: A CSV parser on views
// Level: Advanced
// Shows: a quote-aware CSV reader whose cells are string_view slices of the input, with column lookup by header and checked totals.

#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::string_view_literals;
using namespace hsd::format_literals;

// Reads all of stdin one line at a time with read_line(). Every line, including
// blank lines and a final line without a newline, is kept, and each one
// ends with '\n' so the text can be split on newlines.
hsd::string read_all_input()
{
    hsd::string text;
    auto line = hsd::io::cin().read_line();

    while (line.is_ok())
    {
        text += line.unwrap();
        text += "\n";
        line = hsd::io::cin().read_line();
    }

    return text;
}

hsd::string_view slice(hsd::string_view text, hsd::usize from, hsd::usize to)
{
    if (from >= to || from >= text.length())
    {
        return hsd::string_view{};
    }

    return text.sub_string(from, to - from).unwrap();
}

// One CSV cell. `raw` is the text between the separators without the outer
// quotes. When the cell contained doubled quotes ("") they are still doubled in
// `raw`, and text() produces the unescaped copy.
struct cell
{
    hsd::string_view raw;
    bool has_escaped_quote = false;

    hsd::string text() const
    {
        hsd::string result;

        for (hsd::usize i = 0; i < raw.length(); ++i)
        {
            result.push_back(raw[i]);
            if (has_escaped_quote && raw[i] == '"')
            {
                ++i; // skip the second quote of the pair
            }
        }
        return result;
    }
};

// Splits one line into cells. A cell that starts with '"' runs to the matching
// closing quote, so it may hold commas. Returns false on an unterminated quote.
bool parse_row(hsd::string_view line, hsd::vector<cell>& cells)
{
    hsd::usize pos = 0;

    for (;;)
    {
        cell current;

        if (pos < line.length() && line[pos] == '"')
        {
            hsd::usize start = pos + 1;
            hsd::usize scan = start;
            bool closed = false;

            while (scan < line.length())
            {
                if (line[scan] == '"')
                {
                    if (scan + 1 < line.length() && line[scan + 1] == '"')
                    {
                        current.has_escaped_quote = true;
                        scan += 2;
                        continue;
                    }
                    closed = true;
                    break;
                }
                ++scan;
            }

            if (!closed)
            {
                return false;
            }

            current.raw = slice(line, start, scan);
            pos = scan + 1;
        }
        else
        {
            hsd::usize comma = line.find(',', pos);
            hsd::usize end = comma == hsd::string_view::npos ? line.length() : comma;
            current.raw = slice(line, pos, end);
            pos = end;
        }

        cells.push_back(current);

        if (pos >= line.length())
        {
            return true;
        }

        if (line[pos] != ',')
        {
            return false;
        }
        ++pos;
    }
}

// Parses "12", "12.5" or "0.75" into hundredths. Returns false on anything else.
bool parse_hundredths(hsd::string_view text, hsd::i64& out)
{
    if (text.length() == 0)
    {
        return false;
    }

    hsd::i64 whole = 0;
    hsd::i64 fraction = 0;
    hsd::usize fraction_digits = 0;
    bool seen_dot = false;

    for (hsd::usize i = 0; i < text.length(); ++i)
    {
        char letter = text[i];

        if (letter == '.' && !seen_dot)
        {
            seen_dot = true;
        }
        else if (letter >= '0' && letter <= '9' && !seen_dot)
        {
            whole = whole * 10 + (letter - '0');
        }
        else if (letter >= '0' && letter <= '9' && fraction_digits < 2)
        {
            fraction = fraction * 10 + (letter - '0');
            ++fraction_digits;
        }
        else
        {
            return false;
        }
    }

    for (; fraction_digits < 2; ++fraction_digits)
    {
        fraction *= 10;
    }

    out = whole * 100 + fraction;
    return true;
}

void print_hundredths(const char* label, hsd::i64 value)
{
    hsd::i64 cents = value % 100;
    hsd::println("{}: {}.{}{}"_fmt, label, value / 100, cents < 10 ? "0" : "", cents);
}

hsd::usize find_column(const hsd::vector<cell>& header, hsd::string_view name)
{
    for (hsd::usize i = 0; i < header.size(); ++i)
    {
        if (header[i].raw == name)
        {
            return i;
        }
    }
    return hsd::string_view::npos;
}

int main()
{
    // The input string owns the text. Every cell below is a view into it.
    hsd::string input = read_all_input();
    hsd::string_view text{input.c_str(), input.length()};

    hsd::vector<cell> header;
    hsd::usize line_number = 0;
    hsd::usize good_rows = 0;
    hsd::usize bad_rows = 0;
    hsd::i64 grand_total = 0;
    hsd::usize name_column = 0;
    hsd::usize qty_column = 0;
    hsd::usize price_column = 0;

    hsd::string_view rest = text;
    while (rest.length() != 0)
    {
        hsd::usize newline = rest.find('\n');
        hsd::string_view line = slice(rest, 0, newline == hsd::string_view::npos ? rest.length() : newline);
        rest = newline == hsd::string_view::npos ? hsd::string_view{} : slice(rest, newline + 1, rest.length());

        if (line.length() == 0)
        {
            continue;
        }
        ++line_number;

        hsd::vector<cell> cells;
        if (!parse_row(line, cells))
        {
            hsd::println("line {}: malformed quoting, skipped"_fmt, line_number);
            ++bad_rows;
            continue;
        }

        if (line_number == 1)
        {
            header = cells;
            name_column = find_column(header, "name"_sv);
            qty_column = find_column(header, "qty"_sv);
            price_column = find_column(header, "price"_sv);

            if (name_column == hsd::string_view::npos || qty_column == hsd::string_view::npos
                || price_column == hsd::string_view::npos)
            {
                hsd::println("header needs name, qty and price columns"_fmt);
                return 0;
            }

            hsd::println("columns: {}"_fmt, header.size());
            continue;
        }

        if (cells.size() != header.size())
        {
            hsd::println("line {}: expected {} cells, found {}"_fmt, line_number, header.size(), cells.size());
            ++bad_rows;
            continue;
        }

        hsd::i64 qty = 0;
        hsd::i64 price = 0;
        if (!parse_hundredths(cells[qty_column].raw, qty) || !parse_hundredths(cells[price_column].raw, price))
        {
            hsd::println("line {}: bad number, skipped"_fmt, line_number);
            ++bad_rows;
            continue;
        }

        // qty is in hundredths too, so the product is in ten-thousandths.
        hsd::i64 line_total = qty * price / 100;
        grand_total += line_total;
        ++good_rows;
        hsd::print("{}"_fmt, cells[name_column].text());
        print_hundredths(" costs", line_total);
    }

    hsd::println("rows: {} good, {} skipped"_fmt, good_rows, bad_rows);
    print_hundredths("total", grand_total);

    return 0;
}
