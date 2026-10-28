// Title: File round trip
// Level: Intermediate
// Shows: io::load_file for writing a new file and reading it back, flush, read_chunk, set_data,
// read_line over a file whose blank line and final line are kept, and a print longer than the io buffer.

#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

int main()
{
    // The file does not exist yet. Opening it for writing creates it.
    const char* path = "created_scores.txt";

    {
        auto opened_for_write = hsd::io::load_file(path, hsd::io_options::write);
        if (opened_for_write.is_ok() == false)
        {
            hsd::println("cannot open the file for writing"_fmt);
            return 0;
        }

        hsd::io file = opened_for_write.unwrap();

        // print only fills the buffer. flush writes it to the file.
        file.print("{} {} {}\n"_fmt, 10, 20, 30);
        file.print("{} {} {}\n"_fmt, 40, 50, 60);
        file.flush().unwrap();
        hsd::println("wrote two rows"_fmt);
    }

    // Opening a file that does not exist is an error, not a crash.
    auto missing = hsd::io::load_file("no_such_file.txt");
    hsd::println("missing file opened: {}"_fmt, missing.is_ok() ? "true" : "false");

    auto opened = hsd::io::load_file(path, hsd::io_options::read);
    if (opened.is_ok() == false)
    {
        hsd::println("cannot reopen the file"_fmt);
        return 0;
    }

    hsd::io file = opened.unwrap();

    // read_chunk copies the file contents into the stream buffer.
    file.read_chunk().unwrap();
    hsd::println("bytes read: {}"_fmt, file.get_stream_data().length());

    // set_data parses the first fields, and the rest stays in the buffer.
    hsd::i32 a = 0, b = 0, c = 0;
    file.set_data(a, b, c).unwrap();
    hsd::println("first row sum: {}"_fmt, a + b + c);

    file.set_data(a, b, c).unwrap();
    hsd::println("second row sum: {}"_fmt, a + b + c);

    // read_line returns each line without its newline. The blank line in the
    // middle is kept as empty text, and the final line has no newline at all.
    auto notes = hsd::io::load_file("no_final_newline.txt", hsd::io_options::read).unwrap();
    hsd::usize lines = 0;
    auto line = notes.read_line();
    while (line.is_ok())
    {
        hsd::println("[{}]"_fmt, line.unwrap());
        ++lines;
        line = notes.read_line();
    }
    hsd::println("lines: {}"_fmt, lines);

    // One print of 1500 characters is longer than the 1024-character buffer io starts
    // with. The buffer grows, so the whole row reaches the file.
    hsd::string cell;
    for (hsd::usize index = 0; index < 1500; ++index)
    {
        cell.push_back('x');
    }

    {
        auto wide = hsd::io::load_file("long_row.txt", hsd::io_options::write).unwrap();
        wide.print("<{}>\n"_fmt, cell).flush().unwrap();
    }

    auto wide_file = hsd::io::load_file("long_row.txt", hsd::io_options::read).unwrap();
    auto wide_row = wide_file.read_line();

    if (wide_row.is_ok())
    {
        hsd::println("long row length: {}"_fmt, wide_row.unwrap().length());
    }

    return 0;
}
