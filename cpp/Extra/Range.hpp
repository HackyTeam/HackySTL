#pragma once

#include "../Types.hpp"

namespace hsd
{
    class RangeIterator {
    private:
        size_t _val = 0;
        
    public:
        constexpr RangeIterator() = default;

        constexpr RangeIterator(size_t val)
            : _val{val}
        {}

        constexpr RangeIterator& operator=(size_t val) {
            _val = val;
            return *this;
        }

        constexpr RangeIterator& operator++() {
            _val++;
            return *this;
        }

        constexpr RangeIterator operator-(const RangeIterator& rhs) const   { return {_val - rhs._val}; }
        constexpr const size_t operator*() const                            { return _val;              }
        constexpr bool operator!=(const RangeIterator& rhs) const           { return _val != rhs._val;  }
        constexpr bool operator==(const RangeIterator& rhs) const           { return _val == rhs._val;  }
        constexpr bool operator<(const RangeIterator& rhs) const            { return _val <  rhs._val;  }
        constexpr bool operator>(const RangeIterator& rhs) const            { return _val != rhs._val;  }
        constexpr bool operator<=(const RangeIterator& rhs) const           { return _val <= rhs._val;  }
        constexpr bool operator>=(const RangeIterator& rhs) const           { return _val >= rhs._val;  }
        constexpr bool operator!=(size_t rhs) const                         { return _val != rhs;       }
        constexpr bool operator==(size_t rhs) const                         { return _val == rhs;       }
        constexpr bool operator<(size_t rhs) const                          { return _val <  rhs;       }
        constexpr bool operator>(size_t rhs) const                          { return _val >  rhs;       }
        constexpr bool operator<=(size_t rhs) const                         { return _val <= rhs;       }
        constexpr bool operator>=(size_t rhs) const                         { return _val >= rhs;       }
    };

    class Range {
    private:
        RangeIterator _begin, _end;
        
    public:
        constexpr Range(size_t first, size_t last)
            : _begin{first}, _end{last}
        {}

        constexpr const RangeIterator& begin() const    { return _begin; }
        constexpr const RangeIterator& end() const      { return _end;   }
    };
}