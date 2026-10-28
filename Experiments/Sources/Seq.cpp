#include <Types.hpp>
#include <type_traits>
#include <Io.hpp>

/*template <typename T, T... Ints>
struct integer_sequence
{
    static constexpr hsd::usize size() noexcept
    {
        return sizeof...(Ints);
    }
};

template <typename T, T Step, T To, T Next, T... Vals>
struct make_integer_sequence_helper
    : std::conditional_t<Next >= To, integer_sequence<T, Vals...>, 
    make_integer_sequence_helper<T, Step, To, Next + Step, Next, Vals...>>
{};*/

template < typename T, T... Ints >
struct integer_sequence
{
    static constexpr hsd::usize size() noexcept
    {
        return sizeof...(Ints);
    }
};

template < typename T, T Step, T To, T Prev, T... Next >
struct make_integer_sequence_helper
    : hsd::conditional_t< Prev == To, integer_sequence< T, Next... >, 
    make_integer_sequence_helper< T, Step, To, Prev - Step, Prev - Step, Next... >>
{};

template <hsd::usize Integer, hsd::usize Limit>
using gen_multiplies = make_integer_sequence_helper<hsd::usize, Integer, Integer, (Limit / Integer + 1) * Integer>;
using namespace hsd::format_literals;

static void print_num(size_t S)
{
    hsd::print("{} "_fmt, S);
}

template <typename T, T... ints>
static void print_sequence(integer_sequence<T, ints...> int_seq)
{
    hsd::print("The sequence of size {}: "_fmt, int_seq.size());
    
    (print_num(ints), ...);
    
    hsd::print("\n"_fmt);
}

int main()
{
    print_sequence(gen_multiplies<1, 26>{});
}