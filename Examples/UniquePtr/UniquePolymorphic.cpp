// Title: Polymorphic ownership
// Level: Intermediate
// Shows: unique_ptr to a base class, a factory, a vector of owners, and handing ownership between containers.

#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>
#include <Wrappers/UniquePtr.hpp>

using namespace hsd::format_literals;

class animal
{
public:
    // The base destructor is virtual, so deleting through unique_ptr<animal>
    // runs the derived destructor too.
    virtual ~animal() = default;
    virtual const char* name() const = 0;
    virtual hsd::i32 legs() const = 0;
};

class dog : public animal
{
public:
    ~dog() override
    {
        hsd::println("  dog destroyed"_fmt);
    }

    const char* name() const override { return "dog"; }
    hsd::i32 legs() const override { return 4; }
};

class bird : public animal
{
public:
    ~bird() override
    {
        hsd::println("  bird destroyed"_fmt);
    }

    const char* name() const override { return "bird"; }
    hsd::i32 legs() const override { return 2; }
};

class snake : public animal
{
public:
    ~snake() override
    {
        hsd::println("  snake destroyed"_fmt);
    }

    const char* name() const override { return "snake"; }
    hsd::i32 legs() const override { return 0; }
};

// A factory returns ownership to the caller. The derived owner converts to the base owner.
static hsd::unique_ptr<animal> make_animal(char kind)
{
    switch (kind)
    {
        case 'd':
            return hsd::make_unique<dog>();
        case 'b':
            return hsd::make_unique<bird>();
        case 's':
            return hsd::make_unique<snake>();
        default:
            return nullptr;
    }
}

int main()
{
    hsd::vector<hsd::unique_ptr<animal>> zoo;

    // 'x' is unknown, so the factory returns an empty owner, which is skipped.
    const char* order = "dbxs";

    for (hsd::usize index = 0; order[index] != '\0'; ++index)
    {
        auto created = make_animal(order[index]);

        if (created == nullptr)
        {
            hsd::println("no animal for '{}'"_fmt, order[index]);
            continue;
        }

        zoo.push_back(hsd::move(created));
    }

    hsd::i32 legs = 0;

    for (hsd::usize index = 0; index < zoo.size(); ++index)
    {
        hsd::println("{} has {} legs"_fmt, zoo[index]->name(), zoo[index]->legs());
        legs += zoo[index]->legs();
    }

    hsd::println("total legs: {}"_fmt, legs);

    // Move one owner into a second container. The zoo slot is left empty.
    hsd::vector<hsd::unique_ptr<animal>> vet;
    vet.push_back(hsd::move(zoo[1]));
    hsd::println("zoo slot 1 empty: {}"_fmt, zoo[1] == nullptr ? "yes" : "no");
    hsd::println("vet has a {}"_fmt, vet[0]->name());

    // Dropping the vet releases the bird.
    hsd::println("releasing the vet"_fmt);
    vet.pop_back();

    hsd::println("end of main"_fmt);
    return 0;
}
