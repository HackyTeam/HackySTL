// Title: Shared dependency graph
// Level: Intermediate
// Shows: nodes shared by several parents through unsafe_shared_ptr, reference counts, and release order.

#include <Containers/String.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>
#include <Wrappers/SharedPtr.hpp>

using namespace hsd::format_literals;

class package;
using package_ptr = hsd::unsafe_shared_ptr<package>;

// A package lists the packages it depends on. A shared dependency exists once,
// and every package that needs it holds an owner. There is no weak pointer in
// this library, so the graph must stay acyclic: a cycle of owners would never be freed.
class package
{
public:
    static inline hsd::i32 alive = 0;

    hsd::string name;
    hsd::vector<package_ptr> dependencies;

    explicit package(const char* label)
        : name{label}
    {
        ++alive;
    }

    ~package()
    {
        --alive;
    }
};

static package_ptr make_package(const char* name)
{
    // The argument is passed as an rvalue: an lvalue first argument is taken for an allocator.
    return hsd::make_unsafe_shared<package>(hsd::move(name));
}

// Depth-first walk that prints each package once its dependencies are listed,
// so a package always comes after everything it needs.
static void print_build_order(package_ptr node, hsd::vector<hsd::string>& done)
{
    for (hsd::usize index = 0; index < done.size(); ++index)
    {
        if (done[index] == node->name)
        {
            return;
        }
    }

    for (hsd::usize index = 0; index < node->dependencies.size(); ++index)
    {
        print_build_order(node->dependencies[index], done);
    }

    done.push_back(node->name);
    hsd::println("  build {}"_fmt, node->name.c_str());
}

int main()
{
    package_ptr core = make_package("core");
    package_ptr gfx = make_package("gfx");

    package_ptr ui = make_package("ui");
    ui->dependencies.push_back(core);
    ui->dependencies.push_back(gfx);

    package_ptr net = make_package("net");
    net->dependencies.push_back(core);

    package_ptr app = make_package("app");
    app->dependencies.push_back(ui);
    app->dependencies.push_back(net);

    // core is held by the local, ui and net; gfx by the local and ui.
    hsd::println("owners: core {} gfx {} ui {} net {} app {}"_fmt,
        core.get_count(), gfx.get_count(), ui.get_count(), net.get_count(), app.get_count());

    hsd::println("build order:"_fmt);
    hsd::vector<hsd::string> done;
    print_build_order(app, done);
    hsd::println("packages alive: {}"_fmt, package::alive);

    // Dropping the local owners changes nothing yet: the graph still holds everything.
    core = nullptr;
    gfx = nullptr;
    ui = nullptr;
    net = nullptr;
    hsd::println("after dropping locals: alive {} app owners {}"_fmt, package::alive, app.get_count());

    // The last owner of app goes: app, then whatever only it kept alive, are freed.
    app = nullptr;
    hsd::println("after dropping app: alive {}"_fmt, package::alive);
    return 0;
}
