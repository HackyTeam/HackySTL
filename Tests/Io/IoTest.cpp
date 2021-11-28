#include <Io.hpp>

int main()
{
    hsd::wsstream<256> str;
    hsd::i32 x, z;
    hsd::f32 y;
    hsd_println(L"hello, {} and other words", 123.2);
    str.write_data<L"hello, {} and other words\n">(hsd::string{"123.2"});
    
    hsd::io::cin().read_line().unwrap().set_data(x, y, z).unwrap();
    hsd_println(L"{x,fg=234,bg=43}, {e,fg=143,bg=78}, {fg=84,bg=193}", x, y, z);
    
    auto file = hsd::io::load_file(
        "Tests/Io/test.txt", hsd::io::options::text::read_write
    ).unwrap();
    
    [[maybe_unused]] auto c = 0;
    file.read_line().unwrap().set_data(c).unwrap();
    file.print<"{} {} {}">("Hello", 12, -3.4).unwrap();
}