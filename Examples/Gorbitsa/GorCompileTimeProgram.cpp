// Title: Gorbitsa compile-time compiler
// Level: Advanced
// Shows: translating a message into Gorbitsa source in a constexpr function, running it at compile time, and branching with B.

#include <Extra/Constexpr/Gorbitsa.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// Constant-expression text builder, as in the stairs example.
template <hsd::usize Capacity>
struct program_text
{
    char data[Capacity]{};
    hsd::usize length = 0;

    constexpr void add(const char* text)
    {
        for (; *text != '\0'; ++text)
        {
            data[length++] = *text;
        }
    }

    constexpr void add_number(unsigned value)
    {
        if (value >= 100)
            data[length++] = static_cast<char>('0' + value / 100);

        if (value >= 10)
            data[length++] = static_cast<char>('0' + (value / 10) % 10);

        data[length++] = static_cast<char>('0' + value % 10);
    }
};

// "Compiles" a message into Gorbitsa: every character becomes "S<code> T".
template <hsd::usize N>
constexpr auto compile_message(const char (&message)[N])
{
    program_text<N * 8> text;

    for (hsd::usize i = 0; i + 1 < N; i++)
    {
        if (i > 0)
            text.add(" ");

        text.add("S");
        text.add_number(static_cast<unsigned char>(message[i]));
        text.add(" T");
    }

    return text;
}

static constexpr char message[] = "Hello, HackySTL!";
static constexpr auto message_program = compile_message(message);

// B n jumps to instruction number n when the accumulator is zero and otherwise
// does nothing. Instructions are numbered from zero:
//   0 S0   1 B4   2 S65  3 T   4 S66  5 T
// With S0 the jump is taken and only 'B' is printed.
static constexpr char skip_source[] = "S0 B4 S65 T S66 T";
// With S1 the jump is not taken and both letters are printed.
static constexpr char fall_source[] = "S1 B4 S65 T S66 T";

template <typename Program>
consteval auto run()
{
    return hsd::run_parser<Program, true, 0, 0>();
}

// Everything below is evaluated by the compiler, so the checks are static_asserts.
constexpr bool same_text(const auto& output, const char* expected)
{
    hsd::usize index = 0;

    for (; expected[index] != '\0'; index++)
    {
        if (index >= output.size() || output[index] != static_cast<unsigned char>(expected[index]))
            return false;
    }

    return index == output.size();
}

int main()
{
    constexpr auto echoed = run<typename hsd::str_to_arr<message_program.data>::type>();
    constexpr auto skipped = run<typename hsd::str_to_arr<skip_source>::type>();
    constexpr auto fallen = run<typename hsd::str_to_arr<fall_source>::type>();

    static_assert(same_text(echoed, message));
    static_assert(same_text(skipped, "B"));
    static_assert(same_text(fallen, "AB"));

    // The generated program can be longer than its output.
    hsd::println("message length: {}"_fmt, sizeof(message) - 1);
    hsd::println("program length: {}"_fmt, message_program.length);
    hsd::print("program starts: "_fmt);

    for (hsd::usize i = 0; i < 12; i++)
    {
        hsd::print("{}"_fmt, message_program.data[i]);
    }

    hsd::println(""_fmt);

    hsd::print("echoed: "_fmt);

    for (auto code : echoed)
    {
        hsd::print("{}"_fmt, static_cast<char>(code));
    }

    hsd::println(""_fmt);

    hsd::print("S0 B4 ...: "_fmt);

    for (auto code : skipped)
    {
        hsd::print("{}"_fmt, static_cast<char>(code));
    }

    hsd::println(""_fmt);

    hsd::print("S1 B4 ...: "_fmt);

    for (auto code : fallen)
    {
        hsd::print("{}"_fmt, static_cast<char>(code));
    }

    hsd::println(""_fmt);
    // O stores the accumulator into memory, R clears it, and G loads it back, so
    // the value that comes out is the one that went in.
    static constexpr char store_source[] = "S66 O3 R G3 T";
    constexpr auto stored = run<typename hsd::str_to_arr<store_source>::type>();
    static_assert(same_text(stored, "B"));
    hsd::print("stored and reloaded: "_fmt);
    for (auto code : stored)
    {
        hsd::print("{}"_fmt, static_cast<char>(code));
    }
    hsd::println(""_fmt);
    return 0;
}
