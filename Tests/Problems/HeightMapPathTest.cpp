#include <cassert>
#include <queue>
#include <string>
#include <vector>

#include <Containers/Vector.hpp>

struct point
{
    int row;
    int column;
};

static int shortest_path(const char* const* rows, int row_count, int column_count)
{
    std::vector<int> distances(
        static_cast<std::size_t>(row_count * column_count), -1
    );
    hsd::vector<int> queue;

    auto index = [column_count](int row, int column)
    {
        return row * column_count + column;
    };

    point start{};
    point finish{};
    for (int row = 0; row < row_count; ++row)
    {
        for (int column = 0; column < column_count; ++column)
        {
            if (rows[row][column] == 'S')
            {
                start = {row, column};
            }
            if (rows[row][column] == 'E')
            {
                finish = {row, column};
            }
        }
    }

    queue.push_back(index(start.row, start.column));
    distances[static_cast<std::size_t>(queue.front())] = 0;
    std::size_t head = 0;
    constexpr int row_offsets[] = {-1, 1, 0, 0};
    constexpr int column_offsets[] = {0, 0, -1, 1};

    auto height = [&](int row, int column)
    {
        const char value = rows[row][column];
        return value == 'S' ? 'a' : (value == 'E' ? 'z' : value);
    };

    while (head < queue.size())
    {
        const int current = queue[head++];
        const int row = current / column_count;
        const int column = current % column_count;
        if (row == finish.row && column == finish.column)
        {
            return distances[static_cast<std::size_t>(current)];
        }

        for (int direction = 0; direction < 4; ++direction)
        {
            const int next_row = row + row_offsets[direction];
            const int next_column = column + column_offsets[direction];
            if (next_row < 0 || next_row >= row_count ||
                next_column < 0 || next_column >= column_count)
            {
                continue;
            }

            const int next = index(next_row, next_column);
            if (distances[static_cast<std::size_t>(next)] != -1)
            {
                continue;
            }
            if (height(next_row, next_column) > height(row, column) + 1)
            {
                continue;
            }

            distances[static_cast<std::size_t>(next)] =
                distances[static_cast<std::size_t>(current)] + 1;
            queue.push_back(next);
        }
    }

    return -1;
}

static int reference_shortest_path(const std::vector<std::string>& rows)
{
    std::vector<int> distances(rows.size() * rows.front().size(), -1);
    std::queue<int> queue;
    int start = 0;
    int finish = 0;
    const int width = static_cast<int>(rows.front().size());

    for (int row = 0; row < static_cast<int>(rows.size()); ++row)
    {
        for (int column = 0; column < width; ++column)
        {
            const char value = rows[row][column];
            if (value == 'S') start = row * width + column;
            if (value == 'E') finish = row * width + column;
        }
    }

    queue.push(start);
    distances[static_cast<std::size_t>(start)] = 0;
    while (!queue.empty())
    {
        const int current = queue.front();
        queue.pop();
        if (current == finish) return distances[static_cast<std::size_t>(current)];

        const int row = current / width;
        const int column = current % width;
        const auto height = [&](int next_row, int next_column)
        {
            const char value = rows[next_row][next_column];
            return value == 'S' ? 'a' : (value == 'E' ? 'z' : value);
        };

        for (const auto [row_offset, column_offset] : std::vector<std::pair<int, int>>{
            {-1, 0}, {1, 0}, {0, -1}, {0, 1}
        })
        {
            const int next_row = row + row_offset;
            const int next_column = column + column_offset;
            if (next_row < 0 || next_row >= static_cast<int>(rows.size()) ||
                next_column < 0 || next_column >= width)
            {
                continue;
            }
            const int next = next_row * width + next_column;
            if (distances[static_cast<std::size_t>(next)] == -1 &&
                height(next_row, next_column) <= height(row, column) + 1)
            {
                distances[static_cast<std::size_t>(next)] =
                    distances[static_cast<std::size_t>(current)] + 1;
                queue.push(next);
            }
        }
    }
    return -1;
}

int main()
{
    constexpr const char* rows[] = {
        "Sabqponm",
        "abcryxxl",
        "accszExk",
        "acctuvwj",
        "abdefghi"
    };
    const std::vector<std::string> reference{
        "Sabqponm", "abcryxxl", "accszExk", "acctuvwj", "abdefghi"
    };

    assert(shortest_path(rows, 5, 8) == 31);
    assert(shortest_path(rows, 5, 8) == reference_shortest_path(reference));
}