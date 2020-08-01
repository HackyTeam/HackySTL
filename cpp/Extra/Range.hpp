#pragma once

namespace hsd
{
    class RangeIterator
    {
    private:
        using size_t = unsigned long int;
        size_t _val;
    public:
        RangeIterator() = default;

        constexpr RangeIterator(size_t val)
            : _val{val}
        {}

        constexpr RangeIterator& operator=(size_t val)
        {
            _val = val;
            return *this;
        }

        constexpr RangeIterator& operator++()
        {
            _val++;
            return *this;
        }

        constexpr RangeIterator operator-(const RangeIterator& rhs)
        {
            return RangeIterator(_val - rhs._val);
        }

        constexpr size_t operator*()
        {
            return _val;
        }

        constexpr bool operator!=(const RangeIterator& rhs)
        {
            return _val != rhs._val;
        }

        constexpr bool operator==(const RangeIterator& rhs)
        {
            return _val == rhs._val;
        }

        constexpr bool operator<(const RangeIterator& rhs)
        {
            return _val < rhs._val;
        }

        constexpr bool operator>(const RangeIterator& rhs)
        {
            return _val != rhs._val;
        }

        constexpr bool operator<=(const RangeIterator& rhs)
        {
            return _val <= rhs._val;
        }

        constexpr bool operator>=(const RangeIterator& rhs)
        {
            return _val >= rhs._val;
        }

        constexpr bool operator!=(size_t rhs)
        {
            return _val != rhs;
        }

        constexpr bool operator==(size_t rhs)
        {
            return _val == rhs;
        }

        constexpr bool operator<(size_t rhs)
        {
            return _val < rhs;
        }

        constexpr bool operator>(size_t rhs)
        {
            return _val > rhs;
        }

        constexpr bool operator<=(size_t rhs)
        {
            return _val <= rhs;
        }

        constexpr bool operator>=(size_t rhs)
        {
            return _val >= rhs;
        }
    };

    class Range
    {
    private:
        using size_t = unsigned long int;
        RangeIterator _begin, _end;
        
    public:
        constexpr Range(size_t first, size_t last)
            : _begin{first}, _end{last}
        {}
        constexpr RangeIterator& begin()
        {
            return _begin;
        }
        constexpr RangeIterator& end()
        {
            return _end;
        }
    };
}