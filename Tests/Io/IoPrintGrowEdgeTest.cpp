#include <cassert>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

#include <Containers/String.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// Regression tests for formatted output longer than the buffer it is written into.
// Output into a dynamic stream or an io handle must grow the buffer. Before the fix,
// output past the buffer's capacity panicked; io starts with a 1024-character buffer.
namespace
{
    // Files live in a fresh temporary directory, so the source tree is never written.
    const std::filesystem::path scratch_directory =
        std::filesystem::temp_directory_path() / "hsd_io_print_grow";
    const std::string output_path = (scratch_directory / "output.txt").string();

    std::string filled(std::size_t count, char fill)
    {
        return std::string(count, fill);
    }

    void check_dynamic_stream_grows()
    {
        // A new stream has no storage, so the first write must allocate it.
        const hsd::string text{filled(3000, 'a').c_str()};
        hsd::sstream stream;
        stream.write_data("{}"_fmt, text);
        assert(std::string(stream.data(), stream.size()) == filled(3000, 'a'));

        // Two values in one call, starting from a small reserve.
        hsd::sstream pieces;
        pieces.reserve(4);
        pieces.write_data("[{}|{}]"_fmt, text, text);
        const std::string expected = "[" + filled(3000, 'a') + "|" + filled(3000, 'a') + "]";
        assert(std::string(pieces.data(), pieces.size()) == expected);

        // Appending keeps the existing text and grows the buffer again.
        pieces.append_data("+{}"_fmt, text);
        assert(pieces.size() == expected.size() + 1 + 3000);
        assert(std::string(pieces.data(), pieces.size()) == expected + "+" + filled(3000, 'a'));
    }

    void check_io_print_grows()
    {
        const hsd::string text{filled(3000, 'b').c_str()};
        {
            // Each call below writes more than the io buffer can hold at once.
            auto file = hsd::io::load_file(output_path.c_str(), hsd::io_options::write).unwrap();
            file.print("{}|{}\n"_fmt, text, text).print("{}\n"_fmt, text).flush().unwrap();
        }

        std::ifstream input{output_path, std::ios::binary};
        const std::string content{std::istreambuf_iterator<char>{input}, std::istreambuf_iterator<char>{}};
        const std::string line = filled(3000, 'b');
        assert(content == line + "|" + line + "\n" + line + "\n");
    }
}

int main()
{
    std::filesystem::create_directories(scratch_directory);
    std::filesystem::remove(output_path);

    check_dynamic_stream_grows();
    check_io_print_grows();

    std::filesystem::remove_all(scratch_directory);
    return 0;
}
