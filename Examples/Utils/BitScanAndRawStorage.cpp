// Title: Bit scans and raw storage
// Level: Advanced
// Shows: countl_zero and countr_zero in constant expressions, and aligned storage built by hand.

#include <Base/Utility.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// The position of the highest set bit, or -1 for zero.
constexpr hsd::i32 highest_bit(hsd::u32 value)
{
    return 31 - hsd::countl_zero(value);
}

// Bits are counted by clearing the lowest set bit each time round.
constexpr hsd::u32 count_bits(hsd::u64 value)
{
    hsd::u32 total = 0;

    while (value != 0)
    {
        hsd::i32 low = hsd::countr_zero(value);
        value &= ~(hsd::u64{1} << low);
        total++;
    }

    return total;
}

static_assert(highest_bit(1) == 0);
static_assert(highest_bit(1000) == 9);
static_assert(highest_bit(0) == -1);
static_assert(count_bits(0xF0F0) == 8);
static_assert(count_bits(~hsd::u64{0}) == 64);
static_assert(hsd::countr_zero(hsd::u16{0x8000}) == 15);

// An object that counts how many instances are alive.
struct tracked
{
    hsd::i32 id;
    static inline hsd::i32 alive = 0;

    explicit tracked(hsd::i32 value) : id(value)
    {
        alive++;
    }

    ~tracked()
    {
        alive--;
    }
};

struct alignas(64) cache_line
{
    hsd::i32 value;
    // Explicit padding fills the 64-byte line, so the compiler adds none (MSVC C4324).
    hsd::u8 padding[60];
};

int main()
{
    hsd::println("highest bit of 1000: {}"_fmt, highest_bit(1000));
    hsd::println("set bits in 0xF0F0: {}"_fmt, count_bits(0xF0F0));
    hsd::println("trailing zeros of 96: {}"_fmt, hsd::countr_zero(hsd::u32{96}));
    hsd::println("leading zeros of 96: {}"_fmt, hsd::countl_zero(hsd::u32{96}));
    hsd::println("leading zeros of a zero byte: {}"_fmt, hsd::countl_zero(hsd::u8{0}));

    // Raw storage is only memory: no object lives there until construct_at
    // builds one, and the destructor must be called by hand.
    constexpr hsd::usize count = 3;
    constexpr hsd::usize bytes = sizeof(tracked) * count;
    void* raw = hsd::new_allocate(bytes, alignof(tracked));

    if (raw == nullptr)
    {
        hsd::println("allocation failed"_fmt);
        return 0;
    }

    tracked* slots = static_cast<tracked*>(raw);

    for (hsd::usize index = 0; index < count; index++)
    {
        hsd::construct_at(slots + index, static_cast<hsd::i32>(index * 10));
    }

    hsd::println("alive after construction: {}"_fmt, tracked::alive);

    hsd::print("ids:"_fmt);
    for (hsd::usize index = 0; index < count; index++)
    {
        hsd::print(" {}"_fmt, slots[index].id);
    }
    hsd::println(""_fmt);

    for (hsd::usize index = 0; index < count; index++)
    {
        slots[index].~tracked();
    }

    hsd::println("alive after destruction: {}"_fmt, tracked::alive);
    hsd::delete_deallocate(raw, bytes, alignof(tracked));

    // Over-aligned requests go through the aligned form of operator new.
    void* aligned = hsd::new_allocate(sizeof(cache_line), alignof(cache_line));

    if (aligned != nullptr)
    {
        bool is_aligned = reinterpret_cast<hsd::usize>(aligned) % alignof(cache_line) == 0;
        hsd::println("64-byte aligned: {}"_fmt, is_aligned ? "true" : "false");
        hsd::delete_deallocate(aligned, sizeof(cache_line), alignof(cache_line));
    }

    return 0;
}
