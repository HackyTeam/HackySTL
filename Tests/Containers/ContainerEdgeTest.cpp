#include <cassert>
#include <string_view>

#include <Containers/Deque.hpp>
#include <Containers/ForwardList.hpp>
#include <Containers/HeapArray.hpp>
#include <Containers/List.hpp>
#include <Containers/Span.hpp>
#include <Containers/Vector.hpp>

// Edge cases of the containers: the out-of-bounds message, reversed erase ranges, array
// assignment into a list that already holds elements, erasing through an empty or foreign
// iterator, a span that takes more than it has, and self move assignment.
namespace
{
    // Counts live objects, so the deque checks below see growth and erasure of objects that
    // are not trivially copyable.
    struct counted
    {
        static inline int live = 0;
        int value = 0;

        counted()
        {
            ++live;
        }

        explicit counted(int new_value)
            : value{new_value}
        {
            ++live;
        }

        counted(const counted& other)
            : value{other.value}
        {
            ++live;
        }

        counted(counted&& other) noexcept
            : value{other.value}
        {
            ++live;
        }

        counted& operator=(const counted& other)
        {
            value = other.value;
            return *this;
        }

        counted& operator=(counted&& other) noexcept
        {
            value = other.value;
            return *this;
        }

        ~counted()
        {
            --live;
        }
    };

    bool is_out_of_bounds(const char* message)
    {
        return std::string_view{message} == "Tried to access an element out of bounds";
    }
}

int main()
{
    // deque: at reports the message for a missing index, and erase_for rejects a reversed range.
    hsd::deque<int> _deque;
    _deque.push_back(10);
    _deque.push_back(20);
    auto _missing = _deque.at(7);
    assert(!_missing.is_ok() && is_out_of_bounds(_missing.unwrap_err().pretty_error()));
    assert(_deque.at(1).is_ok() && _deque.at(1).unwrap().get() == 20);
    auto _second = _deque.begin();
    ++_second;
    assert(!_deque.erase_for(_second, _deque.begin()).is_ok());

    // Erasing a range from the middle keeps the elements on both sides in order.
    hsd::deque<int> _middle_erase;
    for (int _value : {1, 2, 3, 4, 5})
    {
        _middle_erase.push_back(_value);
    }
    auto _from = _middle_erase.begin();
    ++_from;
    auto _to = _from;
    ++_to;
    ++_to;
    assert(_middle_erase.erase_for(_from, _to).is_ok());
    assert(_middle_erase.size() == 3 && _middle_erase[0] == 1 && _middle_erase[1] == 4);

    // Growth of a ring whose elements wrap around the end of the buffer keeps their order.
    hsd::deque<int> _ring;
    for (int _value = 0; _value < 6; ++_value)
    {
        _ring.push_back(_value);
    }
    _ring.pop_front();
    _ring.pop_front();
    _ring.pop_front();
    for (int _value = 6; _value < 40; ++_value)
    {
        _ring.push_back(_value);
    }
    assert(_ring.size() == 37 && _ring[0] == 3 && _ring[36] == 39);

    // Growth and erasure of objects that are not trivially copyable.
    {
        hsd::deque<counted> _objects;
        for (int _value = 0; _value < 20; ++_value)
        {
            _objects.push_back(counted{_value});
        }
        assert(_objects.size() == 20 && _objects[19].value == 19);

        auto _first = _objects.begin();
        ++_first;
        auto _last = _first;
        ++_last;
        ++_last;
        assert(_objects.erase_for(_first, _last).is_ok());
        assert(_objects.size() == 18 && _objects[1].value == 3);
    }
    assert(counted::live == 0);

    // forward_list: assigning an array replaces every element, both from an lvalue and a
    // moved array, and erasing an iterator that does not belong to the list is refused.
    hsd::forward_list<int> _forward;
    _forward.push_back(1);
    _forward.push_back(2);
    _forward.push_back(3);
    int _values[2] = {7, 8};
    _forward = _values;
    assert(_forward.begin() != _forward.end());
    int _more[4] = {4, 5, 6, 9};
    _forward = hsd::move(_more);

    hsd::forward_list<int> _other;
    _other.push_back(42);
    auto _foreign = _forward.erase(_other.begin());
    assert(!_foreign.is_ok());

    // forward_list erase: the first element, a middle element, the last element (which moves
    // the tail), and an iterator that refers to no node.
    hsd::forward_list<int> _chain;
    for (int _value : {1, 2, 3, 4})
    {
        _chain.push_back(_value);
    }
    assert(_chain.erase(_chain.begin()).is_ok());
    auto _middle = _chain.begin();
    ++_middle;
    assert(_chain.erase(_middle).is_ok());
    auto _tail = _chain.begin();
    while (true)
    {
        auto _next = _tail;
        ++_next;
        if (_next == _chain.end())
        {
            break;
        }
        ++_tail;
    }
    assert(_chain.erase(_tail).is_ok());
    assert(!_chain.erase(hsd::forward_list<int>::iterator{}).is_ok());

    // Erasing the only element of a forward_list leaves it empty.
    hsd::forward_list<int> _single;
    _single.push_back(5);
    assert(_single.erase(_single.begin()).is_ok());
    assert(_single.begin() == _single.end());

    // list: the same array assignments, and an erase through a default (null) iterator.
    hsd::list<int> _list;
    _list.push_back(1);
    _list.push_back(2);
    int _list_values[2] = {3, 4};
    _list = _list_values;
    int _list_more[3] = {5, 6, 7};
    _list = hsd::move(_list_more);
    auto _null_erase = _list.erase(hsd::list<int>::const_iterator{});
    assert(!_null_erase.is_ok());

    // span: taking more elements than it has is refused.
    hsd::vector<int> _backing{{1, 2, 3}};
    auto _span = _backing.to_span();
    assert(!_span.take(4).is_ok());
    assert(_span.take(2).is_ok());

    // heap_array: moving an array into itself leaves it unchanged.
    hsd::heap_array<int, 3> _heap{{5, 6, 7}};
    _heap = hsd::move(_heap);
    assert(_heap[0] == 5 && _heap.size() == 3);
    return 0;
}
