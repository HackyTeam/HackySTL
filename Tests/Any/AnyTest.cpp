#include <cassert>
#include <any>

#include <Wrappers/Any.hpp>

struct payload
{
    int number;
    char marker;

    friend bool operator==(const payload&, const payload&) = default;
};

int main()
{
    hsd::any value;
    std::any standard_value;
    assert(!value.has_value());
    assert(!standard_value.has_value());
    assert(!value.holds_type<int>());

    value = 42;
    standard_value = 42;
    assert(value.has_value());
    assert(standard_value.has_value());
    assert(value.holds_type<int>());
    assert(std::any_cast<int>(standard_value) == value.cast_to<int>().unwrap().get());
    assert(value.cast_to<int>().unwrap().get() == 42);
    assert(value.cast_if<int>() != nullptr);
    assert(*value.cast_if<int>() == 42);
    assert(value.cast_if<double>() == nullptr);
    assert(!value.cast_to<double>().is_ok());

    value = 3.5;
    standard_value = 3.5;
    assert(value.holds_type<double>());
    assert(standard_value.type() == typeid(double));
    assert(value.cast_to<double>().unwrap().get() == 3.5);
    assert(std::any_cast<double>(standard_value) == value.cast_to<double>().unwrap().get());
    assert(!value.cast_to<int>().is_ok());

    value.emplace<payload>(17, 'x');
    standard_value.emplace<payload>(17, 'x');
    assert(value.holds_type<payload>());
    assert(std::any_cast<payload>(&standard_value)->number
        == value.cast_to<payload>().unwrap().get().number);
    const auto stored_payload = value.cast_to<payload>().unwrap().get();
    assert(stored_payload.number == 17 && stored_payload.marker == 'x');

    hsd::any copy{value};
    std::any standard_copy{standard_value};
    copy.cast_to<payload>().unwrap().get().number = 99;
    std::any_cast<payload>(&standard_copy)->number = 99;
    assert(value.cast_to<payload>().unwrap().get().number == 17);
    assert(std::any_cast<payload>(standard_value).number == 17);

    hsd::any moved{hsd::move(copy)};
    std::any standard_moved{std::move(standard_copy)};
    assert(moved.holds_type<payload>());
    assert(standard_moved.type() == typeid(payload));
    assert(moved.cast_to<payload>().unwrap().get().marker == 'x');
    assert(std::any_cast<payload>(standard_moved).marker
        == moved.cast_to<payload>().unwrap().get().marker);

    hsd::any assigned;
    std::any standard_assigned;
    assigned = moved;
    standard_assigned = standard_moved;
    assert(assigned.holds_type<payload>());
    assert(std::any_cast<payload>(standard_assigned).number
        == assigned.cast_to<payload>().unwrap().get().number);
    value = hsd::move(assigned);
    standard_value = std::move(standard_assigned);
    assert(value.holds_type<payload>());
    assert(std::any_cast<payload>(standard_value).number
        == value.cast_to<payload>().unwrap().get().number);
    assert(assigned.has_value() == false);
    assigned.reset();
    standard_assigned.reset();
    assert(!assigned.has_value());
    assert(!standard_assigned.has_value());
}
