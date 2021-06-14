#pragma once

#include <stdio.h>
#include <wchar.h>

#include "Pair.hpp"
#include "StringLiteral.hpp"
#include "Concepts.hpp"

namespace hsd
{
    namespace sstream_detail
    {
        template <typename CharT>
        inline void _parse(pair<const CharT*, usize>&, auto&) = delete;

        template <basic_string_literal str>
        requires (IsSame<char, typename decltype(str)::char_type>)
        inline i32 _write(auto, pair<char*, usize>) = delete;

        template <basic_string_literal str>
        requires (IsSame<wchar, typename decltype(str)::char_type>)
        inline i32 _write(auto, pair<wchar*, usize>) = delete;
    } // namespace sstream_detail

    namespace io_detail
    {
        template <usize N, string_literal<N> str>
        inline void _print(auto, FILE*) = delete;

        template <usize N, wstring_literal<N> str>
        inline void _print(auto, FILE*) = delete;
    } // namespace io_detail
} // namespace hsd
