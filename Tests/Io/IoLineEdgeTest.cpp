#include <cassert>
#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

#include <Containers/String.hpp>
#include <Serialize/Io.hpp>
#include <Support/TestSupport.hpp>

using namespace hsd::format_literals;

// Regression tests for line and value reading: the last line of a file, blank
// lines, CRLF endings, lines longer than the first buffer, and values at end of file.
namespace
{
    // Files live in a fresh temporary directory, so the source tree is never written.
    const std::filesystem::path scratch_directory =
        std::filesystem::temp_directory_path() / "hsd_io_line_edge";
    const std::string fixture_path = (scratch_directory / "fixture.txt").string();
    const std::string created_path = (scratch_directory / "created.txt").string();

    // Written with the C library so the fixture's size is not limited by the
    // buffer of `hsd::io`, which the reader under test does not depend on.
    void write_fixture(const std::string& content)
    {
        std::FILE* file = std::fopen(fixture_path.c_str(), "wb");
        assert(file != nullptr);
        assert(std::fwrite(content.data(), 1, content.size(), file) == content.size());
        std::fclose(file);
    }

    hsd::io open_fixture()
    {
        return hsd::io::load_file(fixture_path.c_str(), hsd::io_options::read).unwrap();
    }

    std::vector<std::string> read_every_line(const std::string& content)
    {
        write_fixture(content);
        auto file = open_fixture();

        std::vector<std::string> lines;
        auto line = file.read_line();
        while (line.is_ok())
        {
            lines.push_back(test_support::to_string(line.unwrap()));
            line = file.read_line();
        }
        assert(!line.is_ok());
        return lines;
    }

    void expect_lines(const std::string& content, const std::vector<std::string>& expected)
    {
        const auto actual = read_every_line(content);
        assert(actual.size() == expected.size());
        for (std::size_t index = 0; index < expected.size(); ++index)
        {
            assert(actual[index] == expected[index]);
        }
    }

    void check_lines()
    {
        // The final line without a newline is returned, not dropped.
        expect_lines("a\nb\nc", {"a", "b", "c"});
        expect_lines("a\nb\nc\n", {"a", "b", "c"});

        // Blank lines are kept, including at the end.
        expect_lines("\n\nx\n", {"", "", "x"});
        expect_lines("one\n\n", {"one", ""});

        // A carriage return before the newline is part of the line ending.
        expect_lines("first\r\nsecond\r\n", {"first", "second"});
        expect_lines("last\r", {"last"});

        // Tabs are ordinary characters inside a line.
        expect_lines("tab\tinside\n", {"tab\tinside"});

        // An empty file has no lines.
        expect_lines("", {});

        // A line longer than the first buffer keeps every character.
        const std::string long_line(10000, 'q');
        expect_lines(long_line + "\nend", {long_line, "end"});
    }

    void check_values()
    {
        write_fixture("  12\t-7\n\n  99");
        auto file = open_fixture();
        auto first = file.read_value<hsd::i32>();
        auto second = file.read_value<hsd::i32>();
        auto third = file.read_value<hsd::i32>();
        auto fourth = file.read_value<hsd::i32>();
        assert(first.is_ok() && first.unwrap() == 12);
        assert(second.is_ok() && second.unwrap() == -7);
        assert(third.is_ok() && third.unwrap() == 99);
        assert(!fourth.is_ok());

        // The last word of a file ends at end of file, without a separator.
        write_fixture("alpha beta");
        auto words = open_fixture();
        auto alpha = words.read_value<hsd::string>();
        auto beta = words.read_value<hsd::string>();
        assert(alpha.is_ok() && test_support::to_string(alpha.unwrap()) == "alpha");
        assert(beta.is_ok() && test_support::to_string(beta.unwrap()) == "beta");
        assert(!words.read_value<hsd::string>().is_ok());

        write_fixture("");
        auto empty = open_fixture();
        assert(!empty.read_value<hsd::i32>().is_ok());
        assert(empty.is_eof());
    }

    void check_created_file_is_readable()
    {
        // Files created for writing must be readable afterwards.
        auto file = hsd::io::load_file(created_path.c_str(), hsd::io_options::write).unwrap();
        file.print("ok\n"_fmt).flush().unwrap();

        auto reread = hsd::io::load_file(created_path.c_str(), hsd::io_options::read);
        assert(reread.is_ok());
        auto line = reread.unwrap().read_line();
        assert(line.is_ok() && test_support::to_string(line.unwrap()) == "ok");
    }
}

int main()
{
    std::filesystem::create_directories(scratch_directory);

    check_lines();
    check_values();
    check_created_file_is_readable();

    std::filesystem::remove_all(scratch_directory);
}
