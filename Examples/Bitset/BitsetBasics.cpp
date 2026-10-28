// Title: Bitset basics
// Level: Basic
// Shows: building a bitset, set/reset/flip/test, counting, bit operators and shifts.

#include <Containers/Bitset.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

static const char* yes_no(bool value)
{
    return value ? "true" : "false";
}

int main()
{
    // Built from a number. Bit 0 is the rightmost one when printed.
    hsd::bitset<8> flags{0b0010'0110};
    hsd::println("flags    {}"_fmt, flags);
    hsd::println("size {} count {}"_fmt, flags.size(), flags.count());

    // test() is checked and returns a result, operator[] is not.
    hsd::println("bit 1 {} bit 3 {}"_fmt, yes_no(flags.test(1).unwrap()), yes_no(flags[3]));
    hsd::println("test(8) ok: {}"_fmt, yes_no(flags.test(8).is_ok()));

    flags.set(0).unwrap();
    flags.reset(1);
    flags.flip(7);
    hsd::println("changed  {}"_fmt, flags);

    // The proxy returned by operator[] can be assigned to.
    flags[3] = true;
    hsd::println("proxy    {}"_fmt, flags);

    hsd::bitset<8> mask{0b1111'0000};
    hsd::println("and      {}"_fmt, flags & mask);
    hsd::println("or       {}"_fmt, flags | mask);
    hsd::println("xor      {}"_fmt, flags ^ mask);
    hsd::println("not      {}"_fmt, ~flags);
    hsd::println("shl 2    {}"_fmt, flags << 2);
    hsd::println("shr 3    {}"_fmt, flags >> 3);

    hsd::println("any {} none {} all {}"_fmt,
        yes_no(flags.any()), yes_no(flags.none()), yes_no(flags.all()));

    flags.set();
    hsd::println("all set  {} all: {}"_fmt, flags, yes_no(flags.all()));
    flags.reset();
    hsd::println("cleared  {} none: {}"_fmt, flags, yes_no(flags.none()));

    // Assigning one bit to another writes the value, so a chain of assignments
    // sets every bit in the chain.
    hsd::bitset<8> edge_bits{0};
    edge_bits[2] = edge_bits[1] = edge_bits[0] = true;
    hsd::println("chain bits: {}"_fmt, edge_bits.count());
    return 0;
}
