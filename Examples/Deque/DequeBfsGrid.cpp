// Title: Deque breadth-first search on a grid
// Level: Advanced
// Shows: a deque as a BFS queue, a 0-1 BFS using push_front, and path rebuilding with push_front.

#include <Containers/Deque.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// '#' is a wall, 'S' the start and 'G' the goal.
constexpr hsd::usize width = 10;
constexpr hsd::usize height = 6;
using map_rows = const char* const[height];

constexpr map_rows open_map = {
    "S..#......",
    ".#.#.####.",
    ".#...#....",
    ".####.#.#.",
    "......#.#G",
    ".####...#."
};

// The same map with a wall dropped in front of the goal.
constexpr map_rows sealed_map = {
    "S..#......",
    ".#.#.####.",
    ".#...#....",
    ".####.#.##",
    "......#.#G",
    ".####...#."
};

struct cell
{
    hsd::usize x = 0;
    hsd::usize y = 0;
};

constexpr hsd::usize unreachable = static_cast<hsd::usize>(-1);

cell find_char(const char* const* rows, char wanted)
{
    for (hsd::usize y = 0; y < height; ++y)
    {
        for (hsd::usize x = 0; x < width; ++x)
        {
            if (rows[y][x] == wanted)
            {
                return cell{x, y};
            }
        }
    }
    return cell{};
}

// Calls `visit(next_x, next_y)` for each neighbour in a fixed order: up,
// right, down, left. Walls are skipped unless `through_walls` is set.
template <typename Fn>
void for_each_neighbour(const char* const* rows, cell from, bool through_walls, Fn visit)
{
    auto passable = [&](hsd::usize x, hsd::usize y)
    {
        return through_walls || rows[y][x] != '#';
    };

    if (from.y > 0 && passable(from.x, from.y - 1))
    {
        visit(from.x, from.y - 1);
    }
    if (from.x + 1 < width && passable(from.x + 1, from.y))
    {
        visit(from.x + 1, from.y);
    }
    if (from.y + 1 < height && passable(from.x, from.y + 1))
    {
        visit(from.x, from.y + 1);
    }
    if (from.x > 0 && passable(from.x - 1, from.y))
    {
        visit(from.x - 1, from.y);
    }
}

hsd::vector<hsd::usize> fresh_table()
{
    hsd::vector<hsd::usize> table;
    table.resize(width * height);
    for (hsd::usize idx = 0; idx < width * height; ++idx)
    {
        table[idx] = unreachable;
    }
    return table;
}

// Plain BFS: every step costs one, so the deque works as a FIFO queue.
// Returns the number of steps and fills `previous` with each cell's parent.
hsd::usize bfs(const char* const* rows, cell start, cell goal,
    hsd::vector<hsd::usize>& previous)
{
    auto dist = fresh_table();
    previous = fresh_table();

    hsd::deque<cell> queue;
    dist[start.y * width + start.x] = 0;
    queue.push_back(start);

    while (queue.size() > 0)
    {
        cell current = queue.front();
        queue.pop_front();
        hsd::usize current_id = current.y * width + current.x;

        for_each_neighbour(rows, current, false, [&](hsd::usize nx, hsd::usize ny)
        {
            hsd::usize id = ny * width + nx;
            if (dist[id] == unreachable)
            {
                dist[id] = dist[current_id] + 1;
                previous[id] = current_id;
                queue.push_back(cell{nx, ny});
            }
        });
    }

    return dist[goal.y * width + goal.x];
}

// 0-1 BFS: the cost is the number of walls broken. Stepping onto an open cell
// costs 0 and goes to the FRONT of the deque, stepping into a wall costs 1 and
// goes to the BACK. The deque then stays ordered by cost without a heap.
hsd::usize fewest_walls(const char* const* rows, cell start, cell goal)
{
    auto cost = fresh_table();

    hsd::deque<cell> queue;
    cost[start.y * width + start.x] = 0;
    queue.push_back(start);

    while (queue.size() > 0)
    {
        cell current = queue.front();
        queue.pop_front();
        hsd::usize current_cost = cost[current.y * width + current.x];

        for_each_neighbour(rows, current, true, [&](hsd::usize nx, hsd::usize ny)
        {
            hsd::usize id = ny * width + nx;
            bool wall = rows[ny][nx] == '#';
            hsd::usize next_cost = current_cost + (wall ? 1 : 0);

            if (cost[id] == unreachable || next_cost < cost[id])
            {
                cost[id] = next_cost;
                if (wall)
                {
                    queue.push_back(cell{nx, ny});
                }
                else
                {
                    queue.push_front(cell{nx, ny});
                }
            }
        });
    }

    return cost[goal.y * width + goal.x];
}

int main()
{
    cell start = find_char(open_map, 'S');
    cell goal = find_char(open_map, 'G');

    hsd::vector<hsd::usize> previous;
    hsd::usize steps = bfs(open_map, start, goal, previous);
    hsd::println("open map: shortest path is {} steps"_fmt, steps);

    // Walk the parents back from the goal. push_front puts each cell before
    // the one after it, so the path comes out in start-to-goal order.
    hsd::deque<hsd::usize> path;
    hsd::usize walker = goal.y * width + goal.x;
    while (walker != unreachable)
    {
        path.push_front(walker);
        walker = previous[walker];
    }

    hsd::print("path:"_fmt);
    for (const auto& id : path)
    {
        hsd::print(" ({},{})"_fmt, id % width, id / width);
    }
    hsd::println(""_fmt);

    // Draw the route over the map.
    for (hsd::usize y = 0; y < height; ++y)
    {
        for (hsd::usize x = 0; x < width; ++x)
        {
            char shown = open_map[y][x];
            for (const auto& id : path)
            {
                if (id == y * width + x && shown == '.')
                {
                    shown = '*';
                }
            }
            hsd::print("{}"_fmt, shown);
        }
        hsd::println(""_fmt);
    }

    hsd::println("open map: walls to break: {}"_fmt,
        fewest_walls(open_map, start, goal));

    cell sealed_goal = find_char(sealed_map, 'G');
    hsd::usize sealed_steps = bfs(sealed_map, start, sealed_goal, previous);
    hsd::println("sealed map: reachable: {}"_fmt,
        sealed_steps == unreachable ? "false" : "true");
    hsd::println("sealed map: walls to break: {}"_fmt,
        fewest_walls(sealed_map, start, sealed_goal));

    return 0;
}
