#include "../Headers/Print.hpp"
#include "../Headers/StringView.hpp"

int main()
{
    hsd::i32 x = 132, z = -12;
    hsd::f32 y = -334.2;

    hsd::print<"{x,fg=234,bg=43}, {e,fg=143,bg=78}, {fg=84,bg=193}\n">(x, y, z).unwrap();

    hsd::string_view s = {"Hello, world!", 5};

    hsd::print<"{fg=127,bg=103}\n">(s).unwrap();
}