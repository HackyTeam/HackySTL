#include "../../cpp/Io.hpp"
#include "../../cpp/Tuple.hpp"

template <size_t N>
struct FizzBase
{
    char const* msg;

    bool fizz(int x) const
    {
        if (x % N == 0)
            return (hsd::io::print("{}", msg)), true;
        return false;
    }
};

template <typename... Ts>
struct FizzGame
{
    hsd::tuple<Ts...> bases;

    FizzGame(Ts... b) : bases(static_cast<Ts&&>(b)...) {}

    int x = 0;
    void operator*()
    {
        bool acc = false;
        
        [&]<size_t... I>(hsd::index_sequence<I...>) {
            ((acc |= bases.template get<I>().fizz(x)), ...);
        }(hsd::index_sequence_for<Ts...>());
        
        if (!acc)
            hsd::io::print("{}", x);
        
        hsd::io::print("\n");
    }

    FizzGame& operator++() {
        ++x;
        return *this;
    }

    FizzGame& begin() {return *this; }
};

int main() {
    constexpr size_t game_size = 100;
    auto game = FizzGame(FizzBase<3>{"fizz"}, FizzBase<5>{"buzz"});
    
    for (size_t i = 0; i < game_size; ++i)
        *++game;
}