#pragma once

/// @file SStream.hpp
/// @brief Dynamic and fixed-capacity formatted string streams.
/// @details Combines string storage, format generation, token parsing, and
/// separator-aware input for memory-backed I/O.

#include "Containers/StringView.hpp"
#include "_SStreamDetail.hpp"
#include "FormatGenerator.hpp"
#include "_PrinterImpl.hpp"
#include "../Wrappers/Tuple.hpp"

namespace hsd
{
    /// @brief Formatting and token-parsing layer over a character buffer.
    /// @tparam Base Character storage that supplies contiguous data and size.
    template <typename Base>
    class sstream_wrapper : protected Base
    {
    private:
        /// @brief Storage base exposed through the stream interface.
        using base_type = Base;
        /// @brief Character type of the backing storage.
        using CharT = typename Base::value_type;
        /// @brief Protected size field updated after writes and compaction.
        using Base::_size;

        /// @brief Default token separators used by set_data.
        static constexpr CharT _default_seps[] = {' ', '\t', '\n', '\r', '\0'};
        /// @brief Active separator sequence, terminated by a null character.
        const CharT* _separators = _default_seps;

    protected:
        /// @brief Copies storage from a compatible stream or buffer type.
        /// @param other - Source whose storage is copied into the base.
        template <typename T>
        requires (!IsSame<T, sstream_wrapper>)
        constexpr sstream_wrapper(const T& other)
            : base_type{other}
        {}

        /// @brief Returns the first unwritten position in the buffer.
        /// @return Pointer one past the last written character.
        constexpr CharT* get_current_ptr()
        {
            return data() + size();
        }

        /// @brief Returns the number of characters that can still be written.
        /// @return The remaining capacity in characters.
        constexpr usize get_remaining_capacity() const
        {
            return capacity() - size();
        }

        /// @brief Commits `amount` newly written characters to the logical size.
        /// @param amount - Number of characters to add to the logical size.
        constexpr void advance_size(usize amount)
        {
            this->_size += amount;
        }

        /// @brief Enlarges the storage so that more characters can be written.
        /// @return True when the capacity grew, false when the storage is fixed.
        constexpr bool grow_for_output()
        {
            if constexpr (requires (base_type& _base) { _base.reserve(usize{}); })
            {
                const usize _old_capacity = capacity();
                base_type::reserve(_old_capacity * 2 + 64);
                return capacity() > _old_capacity;
            }
            else
            {
                return false;
            }
        }

        /// @brief Tells whether `err` means that the output did not fit in the buffer.
        /// @param err Error returned by a print function.
        /// @return True for a full buffer, false for any other formatting error.
        static constexpr bool is_full_error(const runtime_error& err)
        {
            using namespace hsd::string_view_literals;
            const string_view _msg{err.pretty_error()};
            return (_msg == "Buffer too small for printing"_sv || 
                    _msg == "Destination too small for conversion"_sv);
        }

        /// @brief Formats one piece at the end of the buffer, growing the storage when it is full.
        /// @tparam fmt Resolved format describing the piece.
        /// @param args Value printed by `fmt`; none for literal text.
        /// @details A format tag that the value cannot print is a compile error. Retries
        /// after each growth step. The arguments are passed again on every retry, which is
        /// safe because the print functions only read them. Value-dependent errors, such
        /// as malformed text, and full fixed-capacity storage still panic through `unwrap`.
        template <auto fmt, typename... Args>
        constexpr void print_piece(Args&&... args)
        {
            static_assert(
                requires (CharT* _dest, usize _size, Args&&... _args)
                {
                    print_to<fmt, CharT>(_dest, _size, hsd::forward<Args>(_args)...);
                },
                "The format tag cannot be printed for this value type"
            );

            while (true)
            {
                auto _res = print_to<fmt, CharT>(
                    get_current_ptr(), get_remaining_capacity(), hsd::forward<Args>(args)...
                );

                if (_res.is_ok())
                {
                    advance_size(_res.unwrap());
                    return;
                }

                if (is_full_error(_res.unwrap_err()) == false || grow_for_output() == false)
                {
                    _res.unwrap();
                }
            }
        }

    public:
        /// @brief Mutable iterator over the backing character storage.
        using iterator = CharT*;
        /// @brief Read-only iterator over the backing character storage.
        using const_iterator = const CharT*;
        
        /// @brief Constructs an empty wrapper using the base storage default.
        constexpr sstream_wrapper() = default;

        constexpr sstream_wrapper(const sstream_wrapper&) = delete;
        constexpr sstream_wrapper& operator=(const sstream_wrapper&) = delete;

        /// @brief Transfers storage and separators from `other`.
        /// @param other Wrapper whose storage is moved.
        constexpr sstream_wrapper(sstream_wrapper&& other)
            : base_type{hsd::move(other)}, _separators{other._separators}
        {}

        /// @brief Transfers storage and the active separator pointer.
        /// @param rhs Wrapper whose storage is moved.
        /// @return `*this` after the transfer.
        constexpr sstream_wrapper& operator=(sstream_wrapper&& rhs)
        {
            base_type::operator=(hsd::move(rhs));
            _separators = rhs._separators;
            return *this;
        }

        /// @brief Parses separated buffer fields into the supplied arguments.
        /// @param args Destination objects assigned from successive fields.
        /// @return Empty success, or a runtime error when fields are missing or
        ///         a field cannot be converted to its destination type.
        template <typename... Args>
        constexpr option_err<runtime_error> set_data(Args&... args)
        {
            // HOW DID YOU NOT CRASH PREVIOUSLY???
            auto _data_set = sstream_detail::
                split_data<sizeof...(Args) + 1>(data(), size(), _separators);

            if (sizeof...(Args) > _data_set.size())
            {
                return runtime_error{"Input too small to parse"};
            }
            else if (sizeof...(Args) == _data_set.size())
            {
                return runtime_error{"Attempted to parse incomplete data"};
            }
            else
            {                
                option_err<runtime_error> _error{};
                tuple<reference<Args>...> _tup = {reference<Args>{args}...};
                
                auto _set_func = [&_error](const auto& data, auto& arg)
                {
                    using sstream_detail::_parse_from;

                    if (_error.is_ok() == true)
                    {
                        _error = _parse_from(data, arg);
                    }
                };

                [&]<usize... Ints>(index_sequence<Ints...>)
                {
                    (_set_func(_data_set[Ints], _tup.template get<Ints>()), ...);
                }(make_index_sequence<sizeof...(Args)>{});

                if (_error.is_ok() == false)
                {
                    return _error.unwrap_err();
                }

                usize _sz = 0;
                const auto* _ptr = _data_set.back().data();

                while (static_cast<usize>(_ptr - data()) < size())
                {
                    data()[_sz] = *_ptr;
                    ++_ptr, ++_sz;
                }

                this->_size = _sz;
            }

            return {};
        }

        /// @brief Lets a callback write into the unused buffer tail.
        /// @param func Callback receiving the write pointer and remaining capacity.
        /// @return Empty success, or an error when the callback returns `-1`.
        template <typename Func>
        requires (InvocableRet<isize, Func, char*, usize>)
        constexpr option_err<runtime_error> buffer_write(Func&& func)
        {
            auto _new_size = func(get_current_ptr(), get_remaining_capacity());

            if (_new_size == -1)
            {
                return runtime_error{"Write failed!"};
            }

            advance_size(static_cast<usize>(_new_size));
            return {};
        }

        /// @brief Removes the first `count` characters and moves the rest to the front of the buffer.
        /// @param count Number of leading characters to remove; clamped to the logical size.
        constexpr void drop_front(usize count)
        {
            if (count > size())
            {
                count = size();
            }

            const usize _remaining = size() - count;
            for (usize _index = 0; _index < _remaining; ++_index)
            {
                data()[_index] = data()[_index + count];
            }

            this->_size = _remaining;
        }

        /// @brief Parses the next line, which ends at a newline.
        /// @param arg Destination object assigned from the line's text.
        /// @return Empty success, or a runtime error when no complete line is
        /// buffered or the text cannot be converted. The line is consumed either way.
        /// @details A single trailing carriage return before the newline is
        /// dropped. Blank lines are returned as empty text.
        constexpr option_err<runtime_error> read_line(auto& arg)
        {
            usize _newline = 0;
            while (_newline < size() && data()[_newline] != static_cast<CharT>('\n'))
            {
                ++_newline;
            }

            if (_newline == size())
            {
                return runtime_error{"Attempted to parse incomplete data"};
            }

            usize _length = _newline;
            if (_length != 0 && data()[_length - 1] == static_cast<CharT>('\r'))
            {
                --_length;
            }

            using sstream_detail::_parse_from;
            auto _res = _parse_from(basic_string_view<CharT>{data(), _length}, arg);

            drop_front(_newline + 1);
            return _res;
        }

        /// @brief Clears the buffer and writes formatted data from the beginning.
        /// @param args Values consumed by the format description.
        template <typename T, typename... Args>
        constexpr void write_data(T&&, Args&&... args)
        {
            this->_size = 0;
            append_data<T, Args...>({}, hsd::forward<Args>(args)...);
        }

        /// @brief Appends formatted data to the current buffer contents.
        /// @param args Values consumed by the format description.
        template <typename T, typename... Args>
        constexpr void append_data(T&&, Args&&... args)
        {
            static constexpr T _func{};
            static constexpr auto fmt = _func();
            using char_type = typename decltype(fmt)::char_type;
            using tup_type = type_tuple<Args...>;

            static_assert(
                is_same<char_type, CharT>::value, 
                "Unsupported character type"
            );

            static constexpr auto _fmt_buf =
                parse_literal<fmt, sizeof...(Args) + 1>().unwrap();

            static constexpr auto _last = _fmt_buf[tup_type::size];
            static constexpr auto _last_fmt = 
                format_literal<char_type, _last.length + 1>
                {
                    .format = {_last.format, _last.length},
                    .base = _last.base
                };


            static_assert(
                _fmt_buf.size() == sizeof...(Args) + 1, 
                "The number of arguments doesn't match"
            );

            if constexpr (sizeof...(Args) != 0)
            {
                tuple _tup = {hsd::forward<Args>(args)...};

                [this, &_tup]<usize... Ints>(index_sequence<Ints...>)
                {
                    const auto _print_func = [this]<auto print_fmt>(auto&& val)
                    {
                        using val_type = decay_t<decltype(val)>;

                        if constexpr (print_fmt.base.tag & print_fmt.base.ovr)
                        {
                            static_assert(
                                !(print_fmt.base.tag & print_fmt.base.style),
                                "Format literal cannot have both"
                                " ovr and style tags"
                            );

                            static_assert(
                                requires (val_type _t) {{_t.pretty_args()} -> IsTuple;},
                                "Object must have a pretty_args() method which returns a tuple."    
                            );

                            static constexpr decltype(print_fmt) _curr_fmt = 
                                {
                                    .format = print_fmt.format,
                                    .base = {
                                        .tag = print_fmt.base.tag & ~print_fmt.base.ovr,
                                        .prec_val = print_fmt.base.prec_val,
                                        .style_val = print_fmt.base.style_val
                                    }
                                };

                            constexpr auto _tup_size = decltype(val.pretty_args())::size();

                            [this, _tup_args = val.pretty_args()]<
                                auto pretty_fmt = val_type::template pretty_fmt<CharT>(), usize... Ints2
                            >(index_sequence<Ints2...>)
                            {
                                using namespace format_literals;

                                print_piece<format<_curr_fmt>()>();

                                append_data(operator""_fmt<pretty_fmt>(), _tup_args.template get<Ints2>()...);
                            }(make_index_sequence<_tup_size>{});
                        }
                        else
                        {
                            print_piece<format<print_fmt>()>(hsd::forward<val_type>(val));
                        }
                    };

                    ((_print_func.template operator()<
                        format_literal<char_type, _fmt_buf[Ints].length + 1>
                        {
                            .format = {_fmt_buf[Ints].format, _fmt_buf[Ints].length},
                            .base = _fmt_buf[Ints].base
                        }
                    >(_tup.template get<Ints>())), ...);
                }(make_index_sequence<sizeof...(Args)>{});

                print_piece<format<_last_fmt>()>();
            }
            else
            {
                print_piece<format<_last_fmt>()>();
            }
        }

        /// @brief Selects the separators used by subsequent parsing operations.
        /// @param separators Null-terminated characters that delimit parsed fields.
        constexpr void set_separators(const CharT* separators)
        {
            _separators = separators;
        }

        /// @brief Returns the active, null-terminated separator sequence.
        /// @return Pointer to the active separator sequence.
        constexpr const CharT* get_separators() const
        {
            return _separators;
        }

        /// @brief Restores whitespace separators for token parsing.
        constexpr void set_default_separators()
        {
            _separators = _default_seps;
        }

        /// @brief Exposes the current buffer as a non-owning string view.
        /// @return A view spanning the initialized portion of the backing buffer.
        constexpr operator basic_string_view<CharT>() const
        {
            return {cbegin(), size()};
        }

        using base_type::push_back;
        using base_type::pop_back;
        using base_type::clear;
        using base_type::resize;
        using base_type::capacity;
        using base_type::size;
        using base_type::data;
        using base_type::begin;
        using base_type::end;
        using base_type::cbegin;
        using base_type::cend;
        using base_type::back;
    };

    /// @brief Owning stream buffer backed by a growable vector.
    /// @tparam CharT Character type written to the buffer.
    /// @tparam Allocator Allocator used by the underlying vector.
    template <typename CharT, typename Allocator = allocator>
    class basic_sstream : public sstream_wrapper<vector<CharT, Allocator>>
    {
    private:
        /// @brief Growable storage type inherited by the stream wrapper.
        using subbase_type = vector<CharT, Allocator>;
        /// @brief Formatting and parsing layer inherited by the stream.
        using base_type = sstream_wrapper<subbase_type>;
        /// @brief Allocator type accepted by the stream constructor.
        using alloc_type = Allocator;
    
    public:
        /// @brief Constructs an empty stream with a default allocator.
        constexpr basic_sstream()
        requires (DefaultConstructible<alloc_type>) = default;

        /// @brief Constructs an empty stream using `alloc`.
        /// @param alloc Allocator copied into the backing vector.
        constexpr basic_sstream(const alloc_type& alloc)
            : base_type{alloc}
        {}

        constexpr basic_sstream(const basic_sstream&) = delete;
        constexpr basic_sstream& operator=(const basic_sstream&) = delete;

        /// @brief Transfers the backing buffer from `other`.
        /// @param other - Stream whose buffer is taken over.
        constexpr basic_sstream(basic_sstream&& other)
            : base_type{hsd::move(other)}
        {}

        /// @brief Transfers the backing buffer from `rhs`.
        /// @param rhs - Stream whose buffer is taken over.
        /// @return A reference to this stream.
        constexpr basic_sstream& operator=(basic_sstream&& rhs)
        {
            base_type::operator=(hsd::move(rhs));
            return *this;
        }

        using subbase_type::reserve;
        using subbase_type::get_allocator;
    };

    /// @brief Fixed-capacity stream buffer backed by a static vector.
    /// @tparam CharT Character type written to the buffer.
    /// @tparam N Maximum number of stored characters.
    /// @details Fixed storage cannot grow. Printing writes a terminator after the
    /// text, so the stored text can hold at most `N - 1` characters; a print that
    /// would need more panics. Dynamic streams grow instead.
    template <typename CharT, usize N>
    class static_basic_sstream : public sstream_wrapper<static_vector<CharT, N>>
    {
    private:
        /// @brief Fixed-capacity storage type inherited by the stream wrapper.
        using subbase_type = static_vector<CharT, N>;
        /// @brief Formatting and parsing layer inherited by the stream.
        using base_type = sstream_wrapper<subbase_type>;
    
    public:
        /// @brief Constructs an empty fixed-capacity stream.
        constexpr static_basic_sstream() = default;

        constexpr static_basic_sstream(const static_basic_sstream&) = delete;
        constexpr static_basic_sstream& operator=(const static_basic_sstream&) = delete;

        /// @brief Transfers the fixed buffer from `other`.
        /// @param other - Stream whose buffer is taken over.
        constexpr static_basic_sstream(static_basic_sstream&& other)
            : base_type{hsd::move(other)}
        {}

        /// @brief Transfers the fixed buffer from `rhs`.
        /// @param rhs - Stream whose buffer is taken over.
        /// @return A reference to this stream.
        constexpr static_basic_sstream& operator=(static_basic_sstream&& rhs)
        {
            base_type::operator=(hsd::move(rhs));
            return *this;
        }
    };
    
    /// @brief Narrow-character dynamically allocated stream.
    using sstream = basic_sstream<char>;
    /// @brief Wide-character dynamically allocated stream.
    using wsstream = basic_sstream<wchar>;
    /// @brief UTF-8 dynamically allocated stream.
    using u8sstream = basic_sstream<char8>;
    /// @brief UTF-16 dynamically allocated stream.
    using u16sstream = basic_sstream<char16>;
    /// @brief UTF-32 dynamically allocated stream.
    using u32sstream = basic_sstream<char32>;

    /// @brief Narrow stream using the buffered allocator.
    using buffered_sstream = basic_sstream<char, hsd::buffered_allocator>;
    /// @brief Wide stream using the buffered allocator.
    using buffered_wsstream = basic_sstream<wchar, hsd::buffered_allocator>;
    /// @brief UTF-8 stream using the buffered allocator.
    using buffered_u8sstream = basic_sstream<char8, hsd::buffered_allocator>;
    /// @brief UTF-16 stream using the buffered allocator.
    using buffered_u16sstream = basic_sstream<char16, hsd::buffered_allocator>;
    /// @brief UTF-32 stream using the buffered allocator.
    using buffered_u32sstream = basic_sstream<char32, hsd::buffered_allocator>;

    /// @brief Narrow fixed-capacity stream alias.
    template <usize N>
    using static_sstream = static_basic_sstream<char, N>;

    /// @brief Wide fixed-capacity stream alias.
    template <usize N>
    using static_wsstream = static_basic_sstream<wchar, N>;

    /// @brief UTF-8 fixed-capacity stream alias.
    template <usize N>
    using static_u8sstream = static_basic_sstream<char8, N>;

    /// @brief UTF-16 fixed-capacity stream alias.
    template <usize N>
    using static_u16sstream = static_basic_sstream<char16, N>;

    /// @brief UTF-32 fixed-capacity stream alias.
    template <usize N>
    using static_u32sstream = static_basic_sstream<char32, N>;
} // namespace hsd
