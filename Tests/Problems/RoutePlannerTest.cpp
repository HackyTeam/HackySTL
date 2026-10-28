#include <algorithm>
#include <cassert>

#include <Containers/Vector.hpp>
#include <Base/Result.hpp>

struct route_edge
{
    hsd::usize destination;
    hsd::i64 cost;
};

struct route_result
{
    hsd::i64 cost;
    hsd::vector<hsd::usize> stops;
};

using route_graph = hsd::vector<hsd::vector<route_edge>>;

static auto find_route(
    const route_graph& graph,
    hsd::usize start,
    hsd::usize finish)
    -> hsd::result<route_result, hsd::runtime_error>
{
    if (start >= graph.size() || finish >= graph.size())
    {
        return hsd::runtime_error{"Route endpoint is outside the graph"};
    }

    constexpr hsd::i64 infinity = static_cast<hsd::i64>(1) << 60;
    hsd::vector<hsd::i64> distances;
    hsd::vector<hsd::usize> previous;
    hsd::vector<bool> visited;
    for (hsd::usize node = 0; node < graph.size(); ++node)
    {
        distances.push_back(infinity);
        previous.push_back(graph.size());
        visited.push_back(false);
    }
    distances[start] = 0;

    for (hsd::usize step = 0; step < graph.size(); ++step)
    {
        hsd::usize current = graph.size();
        for (hsd::usize node = 0; node < graph.size(); ++node)
        {
            if (!visited[node] && distances[node] != infinity &&
                (current == graph.size() || distances[node] < distances[current]))
            {
                current = node;
            }
        }

        if (current == graph.size())
        {
            break;
        }
        if (current == finish)
        {
            break;
        }

        visited[current] = true;
        for (const auto& edge : graph[current])
        {
            if (edge.cost < 0)
            {
                return hsd::runtime_error{"Dijkstra requires nonnegative costs"};
            }

            const hsd::i64 candidate = distances[current] + edge.cost;
            if (candidate < distances[edge.destination])
            {
                distances[edge.destination] = candidate;
                previous[edge.destination] = current;
            }
        }
    }

    if (distances[finish] == infinity)
    {
        return hsd::runtime_error{"Destination is unreachable"};
    }

    hsd::vector<hsd::usize> stops;
    for (hsd::usize node = finish; node != graph.size(); node = previous[node])
    {
        stops.push_back(node);
        if (node == start)
        {
            break;
        }
    }
    std::reverse(stops.begin(), stops.end());
    return route_result{distances[finish], hsd::move(stops)};
}

static void add_edge(route_graph& graph, hsd::usize from, hsd::usize to, hsd::i64 cost)
{
    graph[from].push_back({to, cost});
}

int main()
{
    route_graph graph;
    for (int node = 0; node < 7; ++node)
    {
        graph.emplace_back();
    }

    add_edge(graph, 0, 1, 4);
    add_edge(graph, 0, 2, 2);
    add_edge(graph, 2, 1, 1);
    add_edge(graph, 1, 3, 5);
    add_edge(graph, 2, 3, 8);
    add_edge(graph, 2, 4, 10);
    add_edge(graph, 3, 4, 2);
    add_edge(graph, 4, 5, 3);
    add_edge(graph, 3, 5, 20);

    auto route = find_route(graph, 0, 5).unwrap();
    assert(route.cost == 13);
    assert(route.stops.size() == 6);
    assert(route.stops[0] == 0);
    assert(route.stops[1] == 2);
    assert(route.stops[2] == 1);
    assert(route.stops[3] == 3);
    assert(route.stops[4] == 4);
    assert(route.stops[5] == 5);

    auto zero_length = find_route(graph, 3, 3).unwrap();
    assert(zero_length.cost == 0);
    assert(zero_length.stops.size() == 1);
    assert(zero_length.stops[0] == 3);

    assert(!find_route(graph, 0, 6).is_ok());
    assert(!find_route(graph, 0, 7).is_ok());

    add_edge(graph, 5, 6, -1);
    assert(!find_route(graph, 5, 6).is_ok());
}