// Title: Variant visitor
// Level: Intermediate
// Shows: visit with a visitor struct and with a generic lambda, over a vector of shape variants.

#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>
#include <Wrappers/Variant.hpp>

using namespace hsd::format_literals;

struct circle
{
    hsd::f64 radius;
};

struct rectangle
{
    hsd::f64 width;
    hsd::f64 height;
};

struct triangle
{
    hsd::f64 base;
    hsd::f64 height;
};

using shape = hsd::variant<circle, rectangle, triangle>;

// A visitor is any callable that accepts every alternative. A struct with one
// overload per alternative keeps each case separate and readable.
struct area_visitor
{
    hsd::f64 operator()(const circle& value) const
    {
        return 3.14159265 * value.radius * value.radius;
    }

    hsd::f64 operator()(const rectangle& value) const
    {
        return value.width * value.height;
    }

    hsd::f64 operator()(const triangle& value) const
    {
        return 0.5 * value.base * value.height;
    }
};

int main()
{
    hsd::vector<shape> shapes;
    shapes.push_back(shape{circle{1.0}});
    shapes.push_back(shape{rectangle{3.0, 4.0}});
    shapes.push_back(shape{triangle{6.0, 2.5}});
    shapes.push_back(shape{circle{0.5}});

    // visit is a member that returns a result: it fails only when the variant
    // has no value, so unwrap is safe for the shapes built above.
    hsd::f64 total = 0.0;

    for (hsd::usize index = 0; index < shapes.size(); ++index)
    {
        hsd::f64 area = shapes[index].visit(area_visitor{}).unwrap();
        hsd::println("shape {}: area {prec=3}"_fmt, index, area);
        total += area;
    }

    hsd::println("total area: {prec=3}"_fmt, total);

    // A generic lambda with if constexpr handles alternatives one by one,
    // and can name them as text.
    for (hsd::usize index = 0; index < shapes.size(); ++index)
    {
        const char* name = shapes[index].visit([](const auto& value) -> const char*
        {
            using type = hsd::remove_cvref_t<decltype(value)>;

            if constexpr (hsd::IsSame<type, circle>)
            {
                return "circle";
            }
            else if constexpr (hsd::IsSame<type, rectangle>)
            {
                return "rectangle";
            }
            else
            {
                return "triangle";
            }
        }).unwrap();

        hsd::println("shape {} is a {}"_fmt, index, name);
    }

    // A visitor can return nothing. It receives the stored value as an rvalue
    // reference, so a generic lambda takes it as auto&& and can still modify it.
    for (hsd::usize index = 0; index < shapes.size(); ++index)
    {
        shapes[index].visit([](auto&& value)
        {
            if constexpr (hsd::IsSame<hsd::remove_cvref_t<decltype(value)>, circle>)
            {
                value.radius *= 2.0;
            }
        }).unwrap();
    }

    hsd::println("doubled circle: {prec=3}"_fmt, shapes[0].visit(area_visitor{}).unwrap());

    // Counting alternatives by index.
    hsd::usize counts[3]{};

    for (hsd::usize index = 0; index < shapes.size(); ++index)
    {
        counts[shapes[index].index()] += 1;
    }

    hsd::println("circles {} rectangles {} triangles {}"_fmt, counts[0], counts[1], counts[2]);
    return 0;
}
