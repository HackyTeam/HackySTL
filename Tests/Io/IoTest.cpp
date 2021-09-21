#include <Io.hpp>

int main()
{
    hsd::wsstream str{256};
    hsd::i32 x, z;
    hsd::f32 y;
    hsd_println(L"hello, {e,fg=200,bg=62} and other words", 123.2);
    str.write_data<L"hello, {} and other words\n">(hsd::string{"123.2"});
    
    hsd::io::cin().read_line().unwrap().set_data(x, y, z).unwrap();
    hsd_println(L"{}, {}, {}", x, y, z);
    
    auto file = hsd::io::load_file(
        "Tests/Io/test.txt", hsd::io::options::text::read_write
    ).unwrap();
    
    [[maybe_unused]] auto c = file.read_line().unwrap().parse<hsd::i32>();
    file.print<"{} {} {}">("Hello", 12, -3.4).unwrap();
}