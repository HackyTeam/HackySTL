#include <cassert>

#include <Containers/Vector.hpp>
#include <Support/AbortSupport.hpp>

// Edge cases of vector and static_vector: the out-of-bounds message, an erase range that
// is reversed, growth from zero capacity, self move, assignment from arrays in each
// capacity case, and the panics of a full or resized static_vector.
namespace
{
    // Counts live objects, so a test can check that assignments construct and destroy.
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

    void check_access_errors()
    {
        hsd::vector<int> _values{{1, 2, 3}};
        auto _missing = _values.at(7);
        assert(!_missing.is_ok());
        assert(_missing.unwrap_err().pretty_error()[0] == 'T');

        // A range that starts after it ends is rejected.
        auto _reversed = _values.erase_for(_values.begin() + 2, _values.begin());
        assert(!_reversed.is_ok());
    }

    void check_growth_and_self_move()
    {
        // Resizing an empty vector with no capacity grows it to hold the new size.
        hsd::vector<int> _grown;
        _grown.resize(5);
        assert(_grown.size() == 5);
        assert(_grown.capacity() >= 5);

        // Moving a vector into itself leaves it unchanged.
        hsd::vector<int> _self{{4, 5}};
        _self = hsd::move(_self);
        assert(_self.size() == 2 && _self[0] == 4 && _self[1] == 5);
    }

    void check_array_assignment()
    {
        // Enough capacity for the new elements and more elements than the vector holds.
        // The new elements are constructed, not assigned into empty storage.
        {
            hsd::vector<counted> _values;
            _values.reserve(4);
            _values.push_back(counted{1});
            counted _source[3] = {counted{7}, counted{8}, counted{9}};

            _values = _source;
            assert(_values.size() == 3);
            assert(_values[0].value == 7 && _values[1].value == 8 && _values[2].value == 9);
            assert(counted::live == 6);
        }
        assert(counted::live == 0);

        // Fewer elements than the vector holds: the extra elements are destroyed.
        {
            hsd::vector<counted> _values;
            _values.reserve(4);
            _values.push_back(counted{1});
            _values.push_back(counted{2});
            _values.push_back(counted{3});
            counted _source[1] = {counted{5}};

            _values = _source;
            assert(_values.size() == 1 && _values[0].value == 5);
            assert(counted::live == 2);
        }
        assert(counted::live == 0);

        // Not enough capacity: the vector is cleared and reserved again, for both the
        // copy and the move assignment from an array.
        {
            hsd::vector<counted> _values;
            counted _source[2] = {counted{11}, counted{12}};
            _values = _source;
            assert(_values.size() == 2 && _values[1].value == 12);

            _values = hsd::move(_source);
            assert(_values.size() == 2 && _values[0].value == 11);
        }
        assert(counted::live == 0);

        // Same size: the elements are assigned in place.
        {
            hsd::vector<int> _values{{1, 2}};
            _values.reserve(4);
            int _source[2] = {8, 9};
            _values = _source;
            assert(_values[0] == 8 && _values[1] == 9);
        }
    }

    void check_static_vector()
    {
        hsd::static_vector<int, 4> _values;
        _values.push_back(1);

        // Growing inside the capacity, with the new elements zero-filled, then shrinking.
        _values.resize(3);
        assert(_values.size() == 3 && _values[2] == 0);
        _values.resize(3);
        _values.resize(1);
        assert(_values.size() == 1 && _values[0] == 1);

        // emplace_back with room left.
        _values.emplace_back(5);
        assert(_values.size() == 2 && _values[1] == 5);
    }
}

int main()
{
    check_access_errors();
    check_growth_and_self_move();
    check_array_assignment();
    check_static_vector();

    // A static_vector cannot grow past its capacity, and that ends the process.
    test_support::expect_abort([]
    {
        hsd::static_vector<int, 2> _full;
        _full.resize(3);
    });
    test_support::expect_abort([]
    {
        hsd::static_vector<int, 1> _full;
        _full.emplace_back(1);
        _full.emplace_back(2);
    });
    return 0;
}
