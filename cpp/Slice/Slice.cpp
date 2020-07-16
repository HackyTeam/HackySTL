#include <stdio.h>
#include <utility>
#include <stdexcept>

class RangeIterator
{
private:
    size_t _val;
public:
    RangeIterator() = default;

    RangeIterator(size_t val)
    {
        _val = val;
    }

    RangeIterator& operator=(size_t val)
    {
        _val = val;
        return *this;
    }
    
    RangeIterator& operator++()
    {
        _val++;
        return *this;
    }

    RangeIterator operator-(const RangeIterator& rhs)
    {
        return RangeIterator(_val - rhs._val);
    }

    size_t operator*()
    {
        return _val;
    }

    bool operator!=(const RangeIterator& rhs)
    {
        return _val != rhs._val;
    }

    bool operator==(const RangeIterator& rhs)
    {
        return _val == rhs._val;
    }
    
    bool operator<(const RangeIterator& rhs)
    {
        return _val < rhs._val;
    }
    
    bool operator>(const RangeIterator& rhs)
    {
        return _val != rhs._val;
    }

    bool operator<=(const RangeIterator& rhs)
    {
        return _val <= rhs._val;
    }

    bool operator>=(const RangeIterator& rhs)
    {
        return _val >= rhs._val;
    }

    bool operator!=(size_t rhs)
    {
        return _val != rhs;
    }

    bool operator==(size_t rhs)
    {
        return _val == rhs;
    }
    
    bool operator<(size_t rhs)
    {
        return _val < rhs;
    }
    
    bool operator>(size_t rhs)
    {
        return _val > rhs;
    }

    bool operator<=(size_t rhs)
    {
        return _val <= rhs;
    }

    bool operator>=(size_t rhs)
    {
        return _val >= rhs;
    }
};

class Range
{
private:
    RangeIterator _begin, _end;
public:
    Range(size_t first, size_t last)
    {
        _begin = first;
        _end = last;
    }
    RangeIterator& begin()
    {
        return _begin;
    }
    RangeIterator& end()
    {
        return _end;
    }
};

template< typename T >
class vector
{
private:
    T *_data = nullptr;
    size_t _size = 0;
    size_t _reserved_size = 0;

    vector(T* data, size_t size)
    {
        _size = size;
        _reserved_size = size * 2;
        _data = new T[_reserved_size];
        std::copy(data, data + size, begin());
    }
public:
    using iterator = T*;
    using const_iterator = const T*;

    vector(size_t size)
    {
        _data = new T[size];
        _reserved_size = size;
    }
    vector() : _size{0}, _reserved_size{1}
    {
        _data = new T[1];
    }
    ~vector()
    {
        if(_data != nullptr)
            delete[] _data;
    }
    vector(std::initializer_list<T> arr)
    {
        _size = arr.size();
        _reserved_size = _size * 2;
        _data = new T[_reserved_size];
        std::copy(arr.begin(), arr.end(), begin());
    }
    vector& operator=(const vector& lhs)
    {
        if(_data != nullptr)
            delete[] _data;

        _data = new T[lhs._reserved_size];
        std::copy(lhs.begin(), lhs.end(), begin());
        _reserved_size = lhs._reserved_size;
        _size = lhs._size;

        return *this;
    }
    vector operator[](Range rng)
    {
        if(rng.begin() > _size && rng.end() > _size)
            throw std::out_of_range("");

        size_t vec_size = *(rng.end() - rng.begin());

        return vector(&_data[*rng.begin()], vec_size);
    }
    T& operator[](size_t index)
    {
        return _data[index];
    }
    T& at(size_t index)
    {
        if(index >= _size)
            throw std::out_of_range("");
        return _data[index];
    }
    void clear()
    {
        if(_data != nullptr)
            delete[] _data;
        _data = new T[1];
        _size = 0;
        _reserved_size = 1;
    }
    void reserve(size_t size)
    {
        _reserved_size = size;
        T *buf = new T[size];
        if(_data != nullptr)
        {
            std::move(begin(), end(), buf);
            delete[] _data;
        }
        _data = buf;
    }
    void push_back(const T& val)
    {
        if(_reserved_size > _size)
        {
            _data[_size] = val;
            _size++;
            return;
        }
        else
        {
            reserve(_size * 2);
            _data[_size] = val;
            _size++;
        }
    }
    void push_back(T&& val)
    {
        if(_reserved_size > _size)
        {
            _data[_size] = std::move(val);
            _size++;
            return;
        }
        else
        {
            reserve(_size * 2);
            _data[_size] = std::move(val);
            _size++;
        }
    }
    template <typename... Args>
    void emplace_back(Args&&... args)
    {
        if(_reserved_size > _size)
        {
            _data[_size].~T();
            new (&_data[_size]) T(std::forward<Args>(args)...);
            _size++;
            return;
        }
        else
        {
            reserve(_size * 2);
            _data[_size].~T();
            new (&_data[_size]) T(std::forward<Args>(args)...);
            _size++;
        }
    }
    void pop_back()
    {
        _data[--_size].~T();
    }
    size_t size()
    {
        return _size;
    }
    size_t capacity()
    {
        return _reserved_size;
    }
    T* data()
    {
        return _data;
    }
    iterator begin()
    {
        return data();
    }
    iterator end()
    {
        return begin() + size();
    }
    const_iterator cbegin()
    {
        return data();
    }
    const_iterator cend()
    {
        return cbegin() + size();
    }
};

int main()
{
    vector<int> e = {1, 2, 3, 4, 5, 6};

    for(auto val : Range(2, 4))
    {
        printf("%d\n", val);
    }

    /*for (auto val : e)
    {
        printf("%d\n", val);
    }*/

    puts("");
    auto e2 = e[Range(1, 3)];

    for (auto val : e2)
    {
        printf("%d\n", val);
    }
    return 0;
}