#include <Io.hpp>
#include <Tuple.hpp>

using namespace hsd::format_literals;

template <hsd::usize N>
struct FizzBase
{
    char const* msg;

    bool fizz(hsd::u32 x) const
    {
        if (x % N == 0)
        {
            hsd::print("{}"_fmt, msg);
            return true;
        }

        return false;
    }
};

template <typename... Ts>
struct FizzGame
{
    hsd::tuple<Ts...> bases;
    hsd::u32 x = 0;

    FizzGame(Ts&&... b) : bases(hsd::move(b)...) {}    

    void operator*()
    {
        bool acc = false;
        
        [&]<size_t... I>(hsd::index_sequence<I...>) {
            ((acc |= bases.template get<I>().fizz(x)), ...);
        }(hsd::index_sequence_for<Ts...>());
        
        if (!acc) hsd::print("{}"_fmt, x);
        
        hsd::print("\n"_fmt);
    }

    FizzGame& operator++() {
        ++x;
        return *this;
    }

    FizzGame& begin() {return *this; }
};

int main() {
    constexpr hsd::usize game_size = 1000;
    auto game = FizzGame{FizzBase<3>{"fizz"}, FizzBase<5>{"buzz"}};
    
    for (hsd::usize i = 0; i < game_size; ++i)
        *++game;
}