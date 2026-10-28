// Title: Task scheduler
// Level: Advanced
// Shows: dependency resolution with priorities, cycle detection and a critical path, using result errors.

#include <Base/Result.hpp>
#include <Containers/String.hpp>
#include <Containers/UnorderedMap.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

struct task
{
    const char* name;
    hsd::i32 priority;
    hsd::i32 duration;
    hsd::vector<const char*> needs;
};

struct plan
{
    hsd::vector<hsd::usize> order;
    hsd::i32 critical_path = 0;
};

static bool same_text(const char* left, const char* right)
{
    return hsd::string{left} == hsd::string{right};
}

static hsd::result<plan, hsd::runtime_error> make_plan(const hsd::vector<task>& tasks)
{
    const hsd::usize count = tasks.size();

    // Resolve dependency names to indices up front, so unknown names fail early.
    hsd::vector<hsd::vector<hsd::usize>> needs;
    for (const auto& item : tasks)
    {
        needs.emplace_back();
        for (const char* wanted : item.needs)
        {
            bool found = false;
            for (hsd::usize other = 0; other < count; ++other)
            {
                if (same_text(tasks[other].name, wanted))
                {
                    needs.back().push_back(other);
                    found = true;
                }
            }
            if (!found)
            {
                return hsd::runtime_error{"task depends on an unknown task"};
            }
        }
    }

    plan result;
    hsd::vector<bool> done;
    hsd::vector<hsd::i32> finish;
    for (hsd::usize index = 0; index < count; ++index)
    {
        done.push_back(false);
        finish.push_back(0);
    }

    // Each round picks the ready task with the highest priority (lowest index on ties).
    for (hsd::usize round = 0; round < count; ++round)
    {
        hsd::usize best = count;
        for (hsd::usize index = 0; index < count; ++index)
        {
            bool ready = !done[index];
            for (const hsd::usize need : needs[index])
            {
                ready = ready && done[need];
            }
            if (ready && (best == count || tasks[index].priority > tasks[best].priority))
            {
                best = index;
            }
        }

        if (best == count)
        {
            return hsd::runtime_error{"dependency cycle detected"};
        }

        hsd::i32 start = 0;
        for (const hsd::usize need : needs[best])
        {
            start = finish[need] > start ? finish[need] : start;
        }
        finish[best] = start + tasks[best].duration;
        if (finish[best] > result.critical_path)
        {
            result.critical_path = finish[best];
        }

        done[best] = true;
        result.order.push_back(best);
    }
    return hsd::move(result);
}

static void run_scenario(const char* title, const hsd::vector<task>& tasks)
{
    hsd::println("{}:"_fmt, title);
    auto outcome = make_plan(tasks);
    if (!outcome.is_ok())
    {
        hsd::println("  error: {}"_fmt, outcome.unwrap_err().pretty_error());
        return;
    }

    auto scheduled = outcome.unwrap();
    hsd::print("  order:"_fmt);
    for (const hsd::usize index : scheduled.order)
    {
        hsd::print(" {}"_fmt, tasks[index].name);
    }
    hsd::println(""_fmt);
    hsd::println("  critical path: {}"_fmt, scheduled.critical_path);
}

int main()
{
    hsd::vector<task> build;
    build.push_back(task{"fetch", 1, 2, {}});
    build.push_back(task{"configure", 5, 1, {{"fetch"}}});
    build.push_back(task{"docs", 9, 4, {{"fetch"}}});
    build.push_back(task{"compile", 3, 6, {{"configure"}}});
    build.push_back(task{"test", 4, 3, {{"compile"}}});
    build.push_back(task{"package", 2, 1, {{"test", "docs"}}});
    run_scenario("build pipeline", build);

    hsd::vector<task> loop;
    loop.push_back(task{"a", 1, 1, {{"c"}}});
    loop.push_back(task{"b", 1, 1, {{"a"}}});
    loop.push_back(task{"c", 1, 1, {{"b"}}});
    run_scenario("cyclic tasks", loop);

    hsd::vector<task> broken;
    broken.push_back(task{"deploy", 1, 1, {{"approval"}}});
    run_scenario("missing dependency", broken);
}
