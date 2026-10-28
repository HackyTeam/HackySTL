// Title: Property map
// Level: Advanced
// Shows: a string-keyed map of any as a typed property bag, with defaults, type errors and overwriting.

#include <Containers/String.hpp>
#include <Containers/UnorderedMap.hpp>
#include <Serialize/Io.hpp>
#include <Wrappers/Any.hpp>

using namespace hsd::format_literals;

// A small settings object: values of any type, found by name.
class property_map
{
private:
    hsd::unordered_map<hsd::string, hsd::any> _items;

public:
    template <typename T>
    void set(const char* name, T value)
    {
        // operator[] creates the entry when it is missing, then assignment
        // replaces whatever type the any held before.
        _items[hsd::string{name}] = hsd::any{hsd::move(value)};
    }

    // Returns a pointer to the stored T, or null when the name is missing
    // or the property holds a different type.
    template <typename T>
    T* find(const char* name)
    {
        auto found = _items.at(hsd::string{name});

        if (!found.is_ok())
        {
            return nullptr;
        }

        return found.unwrap().get().template cast_if<T>();
    }

    template <typename T>
    T get_or(const char* name, T fallback)
    {
        if (T* value = find<T>(name))
        {
            return *value;
        }

        return fallback;
    }

    // Says why a lookup would fail: missing, wrong type or fine.
    template <typename T>
    const char* check(const char* name)
    {
        auto found = _items.at(hsd::string{name});

        if (!found.is_ok())
        {
            return "missing";
        }

        return found.unwrap().get().template holds_type<T>() ? "ok" : "wrong type";
    }

    hsd::usize count() const
    {
        return _items.size();
    }
};

int main()
{
    property_map settings;
    settings.set("width", 1280);
    settings.set("height", 720);
    settings.set("scale", 1.5);
    settings.set("title", hsd::string{"demo window"});
    settings.set("fullscreen", false);

    hsd::println("properties: {}"_fmt, settings.count());
    hsd::println("width x height: {} x {}"_fmt,
        settings.get_or<hsd::i32>("width", 0), settings.get_or<hsd::i32>("height", 0));
    hsd::println("scale: {prec=2}"_fmt, settings.get_or<hsd::f64>("scale", 1.0));
    hsd::println("title: {}"_fmt, settings.get_or<hsd::string>("title", hsd::string{"?"}).c_str());

    // Defaults cover both a missing name and a name stored with another type.
    hsd::println("depth (missing): {}"_fmt, settings.get_or<hsd::i32>("depth", 24));
    hsd::println("scale as int (wrong type): {}"_fmt, settings.get_or<hsd::i32>("scale", -1));

    hsd::println("check width/i32: {}"_fmt, settings.check<hsd::i32>("width"));
    hsd::println("check width/f64: {}"_fmt, settings.check<hsd::f64>("width"));
    hsd::println("check depth/i32: {}"_fmt, settings.check<hsd::i32>("depth"));

    // Overwriting with another type changes the stored type; the count stays the same.
    settings.set("width", 1920.0);
    hsd::println("width now f64: {} (count {})"_fmt,
        settings.check<hsd::f64>("width"), settings.count());

    // Mutate through the returned pointer.
    if (auto* fullscreen = settings.find<bool>("fullscreen"))
    {
        *fullscreen = true;
    }
    hsd::println("fullscreen: {}"_fmt, settings.get_or<bool>("fullscreen", false) ? "on" : "off");
    return 0;
}
