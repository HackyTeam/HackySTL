// Title: Custom key memoization
// Level: Advanced
// Shows: a struct key with its own hasher, memoizing a recursive grid count, copying and reserving.

#include <Containers/UnorderedMap.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

struct cell
{
    hsd::i32 row;
    hsd::i32 col;

    // The map compares keys with ==, so a custom key must provide it.
    constexpr bool operator==(const cell& rhs) const
    {
        return row == rhs.row && col == rhs.col;
    }
};

// A hasher is any type that names its ResultType and offers get_hash.
// It is the third template argument of unordered_map.
struct cell_hash
{
    using ResultType = hsd::usize;

    static constexpr ResultType get_hash(const cell& key)
    {
        return static_cast<hsd::usize>(key.row) * 1'000'003u
            + static_cast<hsd::usize>(key.col);
    }
};

using memo_table = hsd::unordered_map<cell, hsd::u64, cell_hash>;

constexpr hsd::i32 grid_rows = 12;
constexpr hsd::i32 grid_cols = 12;

static bool is_blocked(const cell& position)
{
    // A small wall with a gap, so the answer is not a plain binomial.
    return position.col == 5 && position.row >= 2 && position.row <= 9;
}

struct counter
{
    memo_table memo;
    hsd::usize hits = 0;
    hsd::usize misses = 0;

    // Counts the paths from the origin to `position`, moving down or right.
    hsd::u64 paths_to(const cell& position)
    {
        if (position.row < 0 || position.col < 0 || is_blocked(position))
        {
            return 0;
        }

        if (position.row == 0 && position.col == 0)
        {
            return 1;
        }

        auto cached = memo.at(position);
        if (cached.is_ok())
        {
            ++hits;
            return cached.unwrap().get();
        }

        ++misses;
        const hsd::u64 total = paths_to(cell{position.row - 1, position.col})
            + paths_to(cell{position.row, position.col - 1});
        memo.emplace(position, total);
        return total;
    }
};

int main()
{
    counter work;
    // Reserving up front avoids rehashing while the memo grows.
    work.memo.reserve(grid_rows * grid_cols);

    const hsd::u64 answer = work.paths_to(cell{grid_rows - 1, grid_cols - 1});
    hsd::println("paths to the far corner: {}"_fmt, answer);
    hsd::println("cells cached: {}, misses: {}, hits: {}"_fmt,
        work.memo.size(), work.misses, work.hits);

    // Asking again is answered from the table with no new work.
    const hsd::usize misses_before = work.misses;
    const hsd::u64 repeated = work.paths_to(cell{grid_rows - 1, grid_cols - 1});
    hsd::println("repeat gives the same answer: {}"_fmt, repeated == answer ? "true" : "false");
    hsd::println("repeat added misses: {}"_fmt, work.misses - misses_before);

    // A copy is independent of the original.
    memo_table snapshot{work.memo};
    work.memo.clear();
    hsd::println("snapshot size: {}, original size: {}"_fmt,
        snapshot.size(), work.memo.size());

    auto corner = snapshot.at(cell{grid_rows - 1, grid_cols - 1});
    if (corner.is_ok())
    {
        hsd::println("snapshot still knows the corner: {}"_fmt, corner.unwrap().get());
    }

    // Iterating the copy visits every cached cell, here only those on row 1.
    hsd::u64 row_sum = 0;
    for (const auto& entry : snapshot)
    {
        if (entry.first.row == 1)
        {
            row_sum += entry.second;
        }
    }
    hsd::println("sum of cached values on row 1: {}"_fmt, row_sum);

    return 0;
}
