// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#pragma once

#include <array>
#include <lexy/_detail/assert.hpp>
#include <lexy/_detail/config.hpp>
#include <lexy/encoding.hpp>

namespace lexy::_detail
{
template <std::size_t N, typename CharT>
struct string_literal
{
    std::array<CharT, N> data;

    using char_type = CharT;

    consteval string_literal(const CharT* str) : data{}
    {
        for (auto i = 0u; i != N; ++i)
            data[i] = str[i];
    }

    consteval string_literal(const std::array<CharT, N> str) : data{}
    {
        for (auto i = 0u; i != N; ++i)
            data[i] = str[i];
    }

    consteval string_literal(CharT c) : data{}
    {
        data[0] = c;
    }

    static consteval auto size()
    {
        return N;
    }
};

template <std::size_t N, typename CharT>
string_literal(const CharT (&)[N]) -> string_literal<N - 1, CharT>;
template <typename CharT>
string_literal(CharT) -> string_literal<1, CharT>;

template <string_literal Str>
using char_type_t = typename decltype(Str)::char_type;

template <string_literal str>
struct type_string
{
    template <typename T>
    static consteval auto build() {
        std::array<T, str.size() + 1> result {};
        for (size_t i = 0 ; i < str.size(); i++)
            result[i] = transcode_char<T>(str.data[i]);
        return result;
    }

    using char_type = decltype(str)::char_type;

    static constexpr std::size_t size = str.size();

    template <typename T = char_type>
    static constexpr const T* c_str()
    {
        static constexpr auto result = build<T>();
        return result.data();
    }
};

} // namespace lexy::_detail


