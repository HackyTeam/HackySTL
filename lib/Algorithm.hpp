#pragma once

#include "Concepts.hpp"

namespace hsd
{
    template <typename T>
    concept DigitVector = requires(T a) {
        {a.size()} -> IsSame<usize>;
        {a[0]} -> IsNumber;
    };

    template <typename T, typename Getter>
    concept DigitVectorGetter = requires(T a, Getter getter) {
        {a.size()} -> IsSame<usize>;
        {a[0]};
        {getter(a[0])} -> IsNumber;
    };

    template <typename Vec>
    static constexpr void count_sort(Vec& arr, usize exp)
    {
        Vec _output = {};
        usize _index, _count[10] = {};
    
        if constexpr (requires {_output.resize(arr.size());})
        {
            _output.resize(arr.size());
        }

        for (_index = 0; _index < arr.size(); _index++)
        {
            _count[(arr[_index] / exp) % 10]++;
        }

        for (_index = 1; _index < 10; _index++)
        {
            _count[_index] += _count[_index - 1];
        }

        for (_index = arr.size() - 1; _index > 0; _index--)
        {
            _output[_count[(arr[_index] / exp) % 10] - 1] = arr[_index];
            _count[(arr[_index] / exp) % 10]--;
        }

        _output[_count[(arr[0] / exp) % 10] - 1] = arr[0];
        _count[(arr[0] / exp) % 10]--;
    
        for (_index = 0; _index < arr.size(); _index++)
        {
            swap(arr[_index], _output[_index]);
        }
    }

    template <typename Vec>
    static constexpr void count_sort(Vec& arr, usize exp, auto&& get_digit)
    {
        Vec _output = {};
        usize _index, _count[10] = {};
    
        if constexpr (requires {_output.resize(arr.size());})
        {
            _output.resize(arr.size());
        }

        for (_index = 0; _index < arr.size(); _index++)
        {
            _count[(get_digit(arr[_index]) / exp) % 10]++;
        }
    
        for (_index = 1; _index < 10; _index++)
        {
            _count[_index] += _count[_index - 1];
        }
    
        for (_index = arr.size() - 1; _index > 0; _index--)
        {
            _output[_count[(get_digit(arr[_index]) / exp) % 10] - 1] = get_digit(arr[_index]);
            _count[(get_digit(arr[_index]) / exp) % 10]--;
        }

        _output[_count[(get_digit(arr[0]) / exp) % 10] - 1] = get_digit(arr[0]);
        _count[(get_digit(arr[0]) / exp) % 10]--;
    
        for (_index = 0; _index < arr.size(); _index++)
        {
            swap(arr[_index], _output[_index]);
        }
    }

    template <typename Vec> requires (DigitVector<Vec>)
    static constexpr void radix_sort(Vec& arr, auto&& pred)
    {
        auto _max_val = [&]()
        {
            auto _mx = arr.begin();

            for (auto _iter = arr.begin(); _iter != arr.end(); _iter++)
            {
                if (pred(*_iter, *_mx))
                {
                    _mx = _iter;
                }
            }

            return *_mx;
        }();
    
        for (usize exp = 1; _max_val / exp > 0; exp *= 10)
        {
            count_sort(arr, exp);
        }
    }

    template <typename Vec, typename Getter>
    requires (DigitVectorGetter<Vec, Getter>)
    static constexpr void radix_sort(Vec& arr, auto&& pred, Getter&& get_digit)
    {
        auto _max_val = [&]()
        {
            auto _mx = arr.begin();

            for (auto _iter = arr.begin(); _iter != arr.end(); _iter++)
            {
                if (pred(*_iter, *_mx))
                {
                    _mx = _iter;
                }
            }

            return *_mx;
        }();
    
        for (usize exp = 1; get_digit(_max_val) / exp > 0; exp *= 10)
        {
            count_sort(arr, exp, get_digit);
        }
    }
} // namespace hsd
