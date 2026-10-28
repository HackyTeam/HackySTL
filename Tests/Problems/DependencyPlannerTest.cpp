#include <cassert>

#include <Containers/Vector.hpp>
#include <Base/Result.hpp>

struct task_dependency
{
    hsd::usize prerequisite;
    hsd::usize task;
};

using dependency_list = hsd::vector<task_dependency>;

static auto make_plan(hsd::usize task_count, const dependency_list& dependencies)
    -> hsd::result<hsd::vector<hsd::usize>, hsd::runtime_error>
{
    hsd::vector<hsd::vector<hsd::usize>> dependents;
    hsd::vector<hsd::usize> indegree;
    for (hsd::usize task = 0; task < task_count; ++task)
    {
        dependents.emplace_back();
        indegree.push_back(0);
    }

    for (const auto& dependency : dependencies)
    {
        const hsd::usize prerequisite = dependency.prerequisite;
        const hsd::usize task = dependency.task;
        if (prerequisite >= task_count || task >= task_count)
        {
            return hsd::runtime_error{"Dependency references an unknown task"};
        }
        dependents[prerequisite].push_back(task);
        ++indegree[task];
    }

    hsd::vector<hsd::usize> ready;
    for (hsd::usize task = 0; task < task_count; ++task)
    {
        if (indegree[task] == 0)
        {
            ready.push_back(task);
        }
    }

    hsd::vector<hsd::usize> plan;
    for (hsd::usize head = 0; head < ready.size(); ++head)
    {
        const hsd::usize completed = ready[head];
        plan.push_back(completed);
        for (const hsd::usize dependent : dependents[completed])
        {
            if (--indegree[dependent] == 0)
            {
                ready.push_back(dependent);
            }
        }
    }

    if (plan.size() != task_count)
    {
        return hsd::runtime_error{"Dependency cycle detected"};
    }
    return hsd::move(plan);
}

static hsd::usize position_of(const hsd::vector<hsd::usize>& plan, hsd::usize task)
{
    for (hsd::usize position = 0; position < plan.size(); ++position)
    {
        if (plan[position] == task)
        {
            return position;
        }
    }
    return plan.size();
}

int main()
{
    dependency_list dependencies;
    dependencies.push_back(task_dependency{0, 1});
    dependencies.push_back(task_dependency{0, 2});
    dependencies.push_back(task_dependency{1, 3});
    dependencies.push_back(task_dependency{2, 3});
    dependencies.push_back(task_dependency{3, 5});
    dependencies.push_back(task_dependency{4, 5});

    auto plan = make_plan(6, dependencies).unwrap();
    assert(plan.size() == 6);
    assert(position_of(plan, 0) < position_of(plan, 1));
    assert(position_of(plan, 0) < position_of(plan, 2));
    assert(position_of(plan, 1) < position_of(plan, 3));
    assert(position_of(plan, 2) < position_of(plan, 3));
    assert(position_of(plan, 3) < position_of(plan, 5));
    assert(position_of(plan, 4) < position_of(plan, 5));

    dependency_list cycle;
    cycle.push_back(task_dependency{0, 1});
    cycle.push_back(task_dependency{1, 2});
    cycle.push_back(task_dependency{2, 0});
    assert(!make_plan(3, cycle).is_ok());

    dependency_list invalid;
    invalid.push_back(task_dependency{0, 2});
    assert(!make_plan(2, invalid).is_ok());
    assert(make_plan(0, dependency_list{}).unwrap().size() == 0);
}