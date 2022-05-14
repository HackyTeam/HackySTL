#pragma once

#include "Result.hpp"
#include "Pair.hpp" 

namespace hsd
{
    namespace span_detail
    {
        template <ContiguousForwardIterable T>
        class countable_iterator
        {
        private:
            T _iter;
            usize _count;
        
        public:
            constexpr countable_iterator(T iter, usize count = 0)
                : _iter{iter}, _count{count}
            {}

            constexpr auto& operator++()
            {
                _iter++;
                _count++;
                return *this;
            }

            constexpr auto operator++(i32)
            {
                countable_iterator tmp = *this;
                operator++();
                return tmp;
            }

            constexpr isize operator-(countable_iterator other) const
            {
                return _iter - other._iter;
            }

            constexpr bool operator==(const countable_iterator& rhs) const
            {
                return _iter == rhs._iter;
            }

            constexpr bool operator!=(const countable_iterator& rhs) const
            {
                return _iter != rhs._iter;
            }

            constexpr auto operator*()
            {
                return hsd::pair<decltype(*_iter), usize>
                {
                    *_iter, _count
                };
            }
            
            constexpr decltype(auto) operator*() const
            {
                return hsd::pair<decltype(*_iter), usize>
                {
                    *_iter, _count
                };
            }
        };

        template <ContiguousForwardIterable T>
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

            constexpr isize operator-(iterator other) const
            {
                return _iter - other._iter;
            }

            constexpr bool operator==(const iterator& rhs) const
            {
                return _iter == rhs._iter;
            }

            constexpr bool operator!=(const iterator& rhs) const
            {
                return _iter != rhs._iter;
            }

            constexpr decltype(auto) operator*()
            {
                return *_iter;
            }
            
            constexpr decltype(auto) operator*() const
            {
                return *_iter;
            }
        };
    } // namespace span_detail

    template <ContiguousForwardIterable T>
    class span
    {
    private:
        using iter_type = typename span_detail::iterator<T>;
        usize _view_size = 0;
        iter_type _begin;
        iter_type _end;
        
    public:
        using reference_type = T&;
        using pointer_type = T*;
        using value_type = T;

        constexpr span(const iter_type& begin, 
            const iter_type& end, usize view_size)
            : _view_size{view_size}, _begin{begin}, _end{end}
        {}

        template <IsForwardContainer U>
        constexpr span(U& container)
            : _view_size{container.size()}, 
            _begin{container.begin()}, 
            _end{container.end()}
        {}

        template <IsForwardContainer U>
        constexpr span(const U& container)
            : _view_size{container.size()}, 
            _begin{container.begin()}, 
            _end{container.end()}
        {}

        template <IsForwardContainer U>
        constexpr span(U&& container) = delete;

        constexpr auto drop(usize quantity) const
            -> result<span, runtime_error>
        {
            if(quantity > _view_size)
            {
                return runtime_error{"Dropping out of bounds"};
            }
            else
            {
                usize _index = 0;
                iter_type _result_iter = _begin;

                for (; _index < quantity; _index++, _result_iter++)
                    ;

                return span{_result_iter, _end, _view_size - _index};
            }
        }

        constexpr auto take(usize quantity) const
            -> result<span, runtime_error> 
        {
            if(quantity > _view_size)
            {
                return runtime_error{"Dropping out of bounds"};
            }
            else
            {
                usize _index = 0;
                iter_type _result_iter = _begin;
                
                for(; _index < quantity; _index++, _result_iter++)
                    ;

                return span {
                    _begin, _result_iter, _view_size - _index
                };
            }
        }

        constexpr auto enumerate()
        {
            using span_detail::countable_iterator;
            return span<countable_iterator<iter_type>> {
                {_begin}, {_end}, _view_size
            };
        }

        constexpr usize size() const
        {
            return _view_size;
        }

        constexpr iter_type begin()
        {
            return _begin;
        }

        constexpr const iter_type begin() const
        {
            return _begin;
        }
        
        constexpr iter_type end()
        {
            return _end;
        }
        
        constexpr const iter_type end() const
        {
            return _end;
        }
    };

    template <IsForwardContainer U>
    span(U& container) -> span<decltype(container.end())>;
    template <IsForwardContainer U>
    span(const U& container) -> span<decltype(container.end())>;
} // namespace hsd
