// Title: Game of Life on bit rows
// Level: Advanced
// Shows: one bitset per row, bit-parallel neighbour counting, and a per-cell reference check.

#include <Containers/Bitset.hpp>
#include <Containers/String.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

constexpr hsd::usize width = 16;
constexpr hsd::usize height = 12;

using row = hsd::bitset<width>;
using board = hsd::vector<row>;

// The board wraps around, so a shift must bring the lost bit back on the other side.
static row rotate_left(const row& value)
{
    return (value << 1) | (value >> (width - 1));
}

static row rotate_right(const row& value)
{
    return (value >> 1) | (value << (width - 1));
}

// Adds one 0/1 plane to a three-bit counter kept as three bit planes.
// Every column is counted at the same time by plain bitwise operators.
static void add_plane(row& bit0, row& bit1, row& bit2, const row& plane)
{
    row carry = plane;
    row next = bit0 & carry;
    bit0 ^= carry;
    carry = next;
    next = bit1 & carry;
    bit1 ^= carry;
    bit2 ^= next;
}

static board step_parallel(const board& now)
{
    board next;

    for (hsd::usize y = 0; y < height; ++y)
    {
        const row& above = now[(y + height - 1) % height];
        const row& same = now[y];
        const row& below = now[(y + 1) % height];

        row bit0, bit1, bit2;
        add_plane(bit0, bit1, bit2, rotate_left(above));
        add_plane(bit0, bit1, bit2, above);
        add_plane(bit0, bit1, bit2, rotate_right(above));
        add_plane(bit0, bit1, bit2, rotate_left(same));
        add_plane(bit0, bit1, bit2, rotate_right(same));
        add_plane(bit0, bit1, bit2, rotate_left(below));
        add_plane(bit0, bit1, bit2, below);
        add_plane(bit0, bit1, bit2, rotate_right(below));

        // Neighbour counts in 8 wrap to 0 in three bits, which is never 2 or 3.
        const row two = ~bit2 & bit1 & ~bit0;
        const row three = ~bit2 & bit1 & bit0;
        next.push_back(three | (same & two));
    }

    return next;
}

// The slow way, one cell at a time, used to check the fast one.
static board step_reference(const board& now)
{
    board next;

    for (hsd::usize y = 0; y < height; ++y)
    {
        row result;

        for (hsd::usize x = 0; x < width; ++x)
        {
            hsd::usize around = 0;

            for (hsd::usize dy = 0; dy < 3; ++dy)
            {
                for (hsd::usize dx = 0; dx < 3; ++dx)
                {
                    if (dy == 1 && dx == 1)
                    {
                        continue;
                    }

                    const hsd::usize cy = (y + height + dy - 1) % height;
                    const hsd::usize cx = (x + width + dx - 1) % width;
                    around += now[cy][cx] ? 1 : 0;
                }
            }

            const bool alive = now[y][x];
            result[x] = (around == 3) || (alive && around == 2);
        }

        next.push_back(result);
    }

    return next;
}

static hsd::usize population(const board& now)
{
    hsd::usize total = 0;
    for (const auto& line : now)
    {
        total += line.count();
    }
    return total;
}

static void show(const char* title, const board& now)
{
    hsd::println("{} (alive {})"_fmt, title, population(now));

    for (const auto& line : now)
    {
        hsd::string text;
        // Bit 0 is the leftmost cell of the picture.
        for (hsd::usize x = 0; x < width; ++x)
        {
            text.push_back(line[x] ? '#' : '.');
        }
        hsd::println("{}"_fmt, text);
    }
}

static bool same_board(const board& lhs, const board& rhs)
{
    for (hsd::usize y = 0; y < height; ++y)
    {
        if (lhs[y] != rhs[y])
        {
            return false;
        }
    }
    return true;
}

int main()
{
    board fast;
    for (hsd::usize y = 0; y < height; ++y)
    {
        fast.push_back(row{});
    }

    // A glider near the top left and a blinker near the bottom right.
    // The proxy from operator[] takes a bool; assign each cell from a bool.
    const hsd::usize alive_cells[][2] = {
        {0, 1}, {1, 2}, {2, 0}, {2, 1}, {2, 2}, {8, 11}, {8, 12}, {8, 13}
    };
    for (const auto& cell : alive_cells)
    {
        fast[cell[0]][cell[1]] = true;
    }

    board slow = fast;
    show("generation 0", fast);

    bool agree = true;
    for (hsd::usize generation = 1; generation <= 24; ++generation)
    {
        fast = step_parallel(fast);
        slow = step_reference(slow);
        agree = agree && same_board(fast, slow);

        if (generation == 4 || generation == 24)
        {
            hsd::println(""_fmt);
            show(generation == 4 ? "generation 4" : "generation 24", fast);
        }
    }

    hsd::println(""_fmt);
    hsd::println("fast and reference agree: {}"_fmt, agree ? "true" : "false");

    return 0;
}
