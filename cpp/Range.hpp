#pragma once

#include "Types.hpp"
#include "Concepts.hpp"
#include "Random.hpp"
#include <time.h>

namespace hsd
{
    namespace ranges
    {
        namespace views
        {
            namespace views_detail
            {
                template <ReverseIterable T>
                class reverse_iterator
                {
                private:
                    T _iter;
                
                public:
                    constexpr reverse_iterator(T iter)
                        : _iter{iter}
                    {}

                    constexpr auto& operator++()
                    {
                        _iter--;
                        return *this;
                    }

                    constexpr reverse_iterator operator++(i32)
                    {
                        reverse_iterator tmp = *this;
                        operator++();
                        return tmp;
                    }

                    constexpr bool operator==(const reverse_iterator& rhs) const
                    {
                        return _iter == rhs._iter;
                    }

                    constexpr bool operator!=(const reverse_iterator& rhs) const
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

                template <ForwardIterable T>
                class forward_iterator
                {
                private:
                    T _iter;
                
                public:
                    constexpr forward_iterator(T iter)
                        : _iter{iter}
                    {}

                    constexpr auto& operator++()
                    {
                        _iter++;
                        return *this;
                    }

                    constexpr forward_iterator operator++(i32)
                    {
                        forward_iterator tmp = *this;
                        operator++();
                        return tmp;
                    }

                    constexpr bool operator==(const forward_iterator& rhs) const
                    {
                        return _iter == rhs._iter;
                    }

                    constexpr bool operator!=(const forward_iterator& rhs) const
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

                template <ForwardIterable T>
                class random_iterator
                {
                private:
                    T _iter;
                    T _origin;
                    usize _index;
                    mt19937_64 engine{};
                
                public:
                    constexpr random_iterator(T iter, usize index)
                        : _iter{iter}, _origin{iter}, _index{index}
                    {}

                    constexpr auto& operator++()
                    {
                        _iter = _origin;
                        
                        usize _to = engine.generate<usize>(0u, _index + 1).unwrap();
                        for(usize _from = 0; _from < _to; _from++, _iter++);

                        _index++;
                        return *this;
                    }

                    constexpr random_iterator operator++(i32)
                    {
                        auto tmp = *this;
                        operator++();
                        return operator++();
                    }

                    constexpr friend bool operator==(
                        const random_iterator& lhs, const random_iterator& rhs)
                    {
                        return lhs._index == rhs._index;
                    }

                    constexpr friend bool operator!=(
                        const random_iterator& lhs, const random_iterator& rhs)
                    {
                        return lhs._index != rhs._index; 
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
            } // namespace views_detail

            template <ForwardIterable T>
            class forward
            {
            private:
                views_detail::forward_iterator<T> _begin;
                views_detail::forward_iterator<T> _end;
            
            public:
                template <IsForwardContainer U>
                constexpr forward(const U& container)
                    : _begin{container.begin()}, _end{container.end()}
                {}

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
            
            template <ReverseIterable T>
            class reverse
            {
            private:
                views_detail::reverse_iterator<T> _begin;
                views_detail::reverse_iterator<T> _end;
            
            public:
                template <IsReverseContainer U>
                constexpr reverse(const U& container)
                    : _begin{container.rbegin()}, _end{container.rend()}
                {}

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

            template <ForwardIterable T>
            class random
            {
            private:
                views_detail::random_iterator<T> _begin;
                views_detail::random_iterator<T> _end;
            
            public:
                template <IsForwardContainer U>
                constexpr random(const U& container)
                    : _begin{container.begin(), 0}, 
                    _end{container.end(), container.size()}
                {}

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
            forward(const U& container) -> forward<decltype(container.end())>;
            template <IsReverseContainer U>
            reverse(const U& container) -> reverse<decltype(container.rend())>;
            template <IsForwardContainer U>
            random(const U& container) -> random<decltype(container.end())>;
        } // namespace views
    } // namespace ranges

    namespace views = ranges::views;    
} // namespace hsd
