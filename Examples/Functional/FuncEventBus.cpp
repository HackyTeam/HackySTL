// Title: Event bus
// Level: Intermediate
// Shows: storing hsd::function callbacks in a vector and a map, calling them, and removing a handler.

#include <Containers/String.hpp>
#include <Containers/UnorderedMap.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>
#include <Wrappers/Functional.hpp>

using namespace hsd::format_literals;

// Handlers for named events. Each handler takes the event payload.
class event_bus
{
public:
    using handler = hsd::function<void(hsd::i32)>;

private:
    hsd::unordered_map<hsd::string, hsd::vector<handler>> _handlers;

public:
    void subscribe(const char* event, handler callback)
    {
        _handlers[hsd::string{event}].push_back(hsd::move(callback));
    }

    // Calls every handler of the event and returns how many ran.
    hsd::usize publish(const char* event, hsd::i32 payload)
    {
        auto found = _handlers.at(hsd::string{event});

        if (!found.is_ok())
        {
            return 0;
        }

        auto& list = found.unwrap().get();
        hsd::usize called = 0;

        for (hsd::usize index = 0; index < list.size(); ++index)
        {
            // An emptied slot is skipped: calling it would only return an error.
            if (list[index])
            {
                list[index](payload).unwrap();
                ++called;
            }
        }

        return called;
    }

    // Empties a handler slot by assigning nullptr.
    void unsubscribe(const char* event, hsd::usize slot)
    {
        auto found = _handlers.at(hsd::string{event});

        if (found.is_ok() && slot < found.unwrap().get().size())
        {
            found.unwrap().get()[slot] = nullptr;
        }
    }
};

int main()
{
    event_bus bus;
    hsd::i32 clicks = 0;
    hsd::i32 last_key = 0;

    // Handlers capture whatever state they need.
    bus.subscribe("click", [&clicks](hsd::i32 count) { clicks += count; });
    bus.subscribe("click", [](hsd::i32 count) { hsd::println("click handler saw {}"_fmt, count); });
    bus.subscribe("key", [&last_key](hsd::i32 code) { last_key = code; });

    hsd::println("click ran {} handlers"_fmt, bus.publish("click", 2));
    hsd::println("click ran {} handlers"_fmt, bus.publish("click", 3));
    hsd::println("key ran {} handlers"_fmt, bus.publish("key", 65));
    hsd::println("scroll ran {} handlers"_fmt, bus.publish("scroll", 1));
    hsd::println("clicks: {} last key: {}"_fmt, clicks, last_key);

    bus.unsubscribe("click", 1);
    hsd::println("after unsubscribe: ran {} handlers"_fmt, bus.publish("click", 4));
    hsd::println("clicks: {}"_fmt, clicks);
    return 0;
}
