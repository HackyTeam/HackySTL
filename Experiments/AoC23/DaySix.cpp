#include <Serialize/Io.hpp>
#include <Base/Math.hpp>

static inline auto get_solution(
    hsd::usize time, hsd::usize distance)
{
    auto sqrt_delta = hsd::math::sqrt(time * time - 4. * distance);
    hsd::usize first = (time + sqrt_delta) / 2;
    hsd::usize second = (time - sqrt_delta) / 2;
    return time - 2 * ((first > second) ? second : first) - 1;
}

int main()
{
    using namespace hsd::format_literals;
    auto res = get_solution(46, 214) *
        get_solution(80, 1177) *
        get_solution(78, 1402) *
        get_solution(66, 1024);
    hsd::println("Part 1: {}"_fmt, res);
    res = get_solution(46807866, 214117714021024);
    hsd::println("Part 2: {}"_fmt, res);
}