#include <cassert>

#include <Containers/Bitset.hpp>

using life_board = hsd::bitset<25>;

static constexpr hsd::usize cell_index(int row, int column)
{
    return static_cast<hsd::usize>(row * 5 + column);
}

static void set_alive(life_board& board, int row, int column)
{
    board.set(cell_index(row, column)).unwrap();
}

static bool is_alive(const life_board& board, int row, int column)
{
    return board.test(cell_index(row, column)).unwrap();
}

static life_board advance(const life_board& current)
{
    life_board next;
    for (int row = 0; row < 5; ++row)
    {
        for (int column = 0; column < 5; ++column)
        {
            int neighbors = 0;
            for (int row_offset = -1; row_offset <= 1; ++row_offset)
            {
                for (int column_offset = -1; column_offset <= 1; ++column_offset)
                {
                    if (row_offset == 0 && column_offset == 0)
                    {
                        continue;
                    }

                    const int neighbor_row = row + row_offset;
                    const int neighbor_column = column + column_offset;
                    if (neighbor_row >= 0 && neighbor_row < 5 &&
                        neighbor_column >= 0 && neighbor_column < 5 &&
                        is_alive(current, neighbor_row, neighbor_column))
                    {
                        ++neighbors;
                    }
                }
            }

            const bool alive = is_alive(current, row, column);
            if (neighbors == 3 || (alive && neighbors == 2))
            {
                set_alive(next, row, column);
            }
        }
    }
    return next;
}

int main()
{
    life_board block;
    set_alive(block, 1, 1);
    set_alive(block, 1, 2);
    set_alive(block, 2, 1);
    set_alive(block, 2, 2);
    assert(advance(block) == block);
    assert(advance(advance(block)) == block);

    life_board blinker;
    set_alive(blinker, 1, 2);
    set_alive(blinker, 2, 2);
    set_alive(blinker, 3, 2);

    life_board horizontal;
    set_alive(horizontal, 2, 1);
    set_alive(horizontal, 2, 2);
    set_alive(horizontal, 2, 3);

    assert(advance(blinker) == horizontal);
    assert(advance(horizontal) == blinker);
    assert(blinker.count() == 3);
    assert(!life_board{}.any());
}