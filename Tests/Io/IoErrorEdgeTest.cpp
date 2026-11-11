#include <cassert>
#include <cstdio>
#include <filesystem>
#include <string>

#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// Error results of the file stream: flushing a file opened for reading, reading from a file
// opened for writing, a missing file, and a value that is not a number.
namespace
{
    // Files live in a fresh temporary directory, so the source tree is never written.
    const std::filesystem::path scratch_directory =
        std::filesystem::temp_directory_path() / "hsd_io_error_edge";
    const std::string fixture_path = (scratch_directory / "fixture.txt").string();
    const std::string created_path = (scratch_directory / "created.txt").string();
    const std::string missing_path = (scratch_directory / "missing.txt").string();

    void write_fixture(const std::string& content)
    {
        std::filesystem::create_directories(scratch_directory);
        std::FILE* file = std::fopen(fixture_path.c_str(), "wb");
        assert(file != nullptr);
        assert(std::fwrite(content.data(), 1, content.size(), file) == content.size());
        std::fclose(file);
    }
}

int main()
{
    write_fixture("hello 12 world\n");

    // A file opened for reading cannot be flushed.
    auto _reader = hsd::io::load_file(fixture_path.c_str(), hsd::io_options::read).unwrap();
    auto _flushed = _reader.flush();
    assert(!_flushed.is_ok());

    // A value that is not a number is reported, and the text is left unread.
    auto _word = _reader.read_value<int>();
    assert(!_word.is_ok());

    // A file opened for writing cannot be read from.
    auto _writer = hsd::io::load_file(created_path.c_str(), hsd::io_options::write).unwrap();
    assert(!_writer.read_chunk().is_ok());
    assert(!_writer.read_line_chunk().is_ok());

    // A file that does not exist cannot be opened for reading.
    auto _missing = hsd::io::load_file(missing_path.c_str(), hsd::io_options::read);
    assert(!_missing.is_ok());

    // Flushing a file opened for writing succeeds, and the text is written to it.
    auto _output = hsd::io::load_file(created_path.c_str(), hsd::io_options::write).unwrap();
    _output.print("written\n"_fmt).flush().unwrap();

    // Reading past the end of the text reports the end of the file.
    auto _input = hsd::io::load_file(created_path.c_str(), hsd::io_options::read).unwrap();
    assert(_input.read_line().is_ok());
    assert(!_input.read_line().is_ok());

#if defined(__linux__)
    // A device that refuses writes makes the flush fail with the system message.
    auto _full = hsd::io::load_file("/dev/full", hsd::io_options::write).unwrap();
    _full.print("cannot be stored\n"_fmt);
    assert(!_full.flush().is_ok());
#endif
    return 0;
}
