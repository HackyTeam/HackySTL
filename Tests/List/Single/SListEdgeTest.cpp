#include <cassert>
#include <forward_list>
#include <iterator>
#include <random>
#include <vector>

#include <Containers/ForwardList.hpp>
#include <Support/TestSupport.hpp>

// Regression tests for forward_list erasure at the tail, compared with std::forward_list.
int main()
{
    // Erasing the last element must move the tail, so the next push_back
    // writes into live storage instead of the freed node.
    {
        hsd::forward_list<int> list{{1, 2, 3}};
        auto last = list.begin();
        ++last;
        ++last;
        assert(list.erase(last).is_ok());
        list.push_back(9);
        test_support::expect_sequence(list, std::vector<int>{1, 2, 9});
        assert(list.size() == 3);
    }

    // Erasing the tail repeatedly, then appending, keeps the tail valid.
    {
        hsd::forward_list<int> list{{4, 5}};
        auto second = list.begin();
        ++second;
        assert(list.erase(second).is_ok());
        auto only = list.begin();
        assert(list.erase(only).is_ok());
        list.push_back(7);
        list.push_back(8);
        test_support::expect_sequence(list, std::vector<int>{7, 8});
    }

    // Randomized operations compared with the standard container.
    {
        hsd::forward_list<int> list;
        std::forward_list<int> expected;
        std::vector<int> mirror;
        std::mt19937 engine{7};
        std::uniform_int_distribution<int> choice{0, 4};
        std::uniform_int_distribution<int> value{0, 1000};

        for (int step = 0; step < 4000; ++step)
        {
            const int action = choice(engine);
            if (action == 0 || mirror.empty())
            {
                const int number = value(engine);
                list.push_back(number);
                expected.insert_after(
                    std::next(expected.before_begin(), static_cast<long>(mirror.size())), number);
                mirror.push_back(number);
            }
            else if (action == 1)
            {
                const int number = value(engine);
                list.push_front(number);
                expected.push_front(number);
                mirror.insert(mirror.begin(), number);
            }
            else if (action == 2)
            {
                list.pop_front();
                expected.pop_front();
                mirror.erase(mirror.begin());
            }
            else
            {
                // Erase at a random position, including the tail.
                const auto index = static_cast<std::size_t>(
                    std::uniform_int_distribution<std::size_t>{0, mirror.size() - 1}(engine));

                auto position = list.begin();
                for (std::size_t step_index = 0; step_index < index; ++step_index)
                {
                    ++position;
                }

                assert(list.erase(position).is_ok());
                expected.erase_after(
                    std::next(expected.before_begin(), static_cast<long>(index)));
                mirror.erase(mirror.begin() + static_cast<long>(index));
            }

            assert(list.size() == mirror.size());
        }

        test_support::expect_sequence(list, mirror);
        std::vector<int> standard(expected.begin(), expected.end());
        test_support::expect_sequence(list, standard);
    }
}
