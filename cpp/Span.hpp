#pragma once

#include "Concepts.hpp"
#include "Result.hpp"

namespace hsd
{
    namespace span_detail
    {
        template <ForwardIterable T>
        class iterator
        {
        private:
            T _iter;
        
        public:
            constexpr iterator(T iter)
                : _iter{iter}
            {}

            constexpr auto& operator++()
            {
                _iter++;
                return *this;
            }

            constexpr iterator operator++(i32)
            {
                iterator tmp = *this;
                operator++();
                return tmp;
            }

            constexpr bool operator==(const iterator& rhs) const
            {
                return _iter == rhs._iter;
            }

            constexpr bool operator!=(const iterator& rhs) const
            {
                return _iter != rhs._iter;
            }

            constexpr auto& operator*()
            {
                return *_iter;
            }
            
            constexpr const auto& operator*() const
            {
                return *_iter;
            }
        };
    } // namespace span_detail
    

    template <ForwardIterable T>
    class span
    {
    private:
        using iter_type = typename span_detail::iterator<T>;
        usize _view_size = 0;
        iter_type _begin;
        iter_type _end;
    
        constexpr span(const iter_type& begin, const iter_type& end, usize view_size)
            : _view_size{view_size}, _begin{begin}, _end{end}
        {}
        
    public:
        using value_type = T;

        template <IsForwardContainer U>
        constexpr span(const U& container)
            : _view_size{container.size()}, 
            _begin{container.begin()}, 
            _end{container.end()}
        {}

        constexpr auto drop(usize quantity) const
            -> Result<span, runtime_error>
        {
            if(quantity > _view_size)
            {
                return runtime_error{"Dropping out of bounds"};
            }
            else
            {
                usize _index = 0;
                iter_type _result_iter = _begin;
                for(; _index < quantity; _index++, _result_iter++);
                return span{_result_iter, _end, _view_size - _index};
            }
        }

        constexpr auto take(usize quantity) const
            -> Result<span, runtime_error> 
        {
            if(quantity > _view_size)
            {
                return runtime_error{"Dropping out of bounds"};
            }
            else
            {
                usize _index = 0;
                iter_type _result_iter = _begin;
                for(; _index < quantity; _index++, _result_iter++);
                return span{_begin, _result_iter, _view_size - _index};
            }
        }

        constexpr usize size() const
        {
            return _view_size;
        }

        constexpr auto begin()
        {
            return _begin;
        }

        constexpr const auto begin() const
        {
            return _begin;
        }
        
        constexpr auto end()
        {
            return _end;
        }
        
        constexpr const auto end() const
        {
            return _end;
        }
    };

    template <IsForwardContainer U>
    span(const U& container) -> span<decltype(container.end())>;
} // namespace hsd
