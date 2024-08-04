// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DETAIL_NTTP_STRING_HPP_INCLUDED
#define LEXY_DETAIL_NTTP_STRING_HPP_INCLUDED

#include <lexy/_detail/assert.hpp>
#include <lexy/_detail/config.hpp>
#include <lexy/encoding.hpp>

namespace lexy::_detail
{
// Note: we can't use type_string<auto...>, it doesn't work on older GCC.
template <typename CharT, CharT... Cs>
struct type_string
{
    using char_type = CharT;

    template <template <typename C, C...> typename T>
    using rename = T<CharT, Cs...>;

    static constexpr auto size = sizeof...(Cs);

    template <typename T = char_type>
    static constexpr T c_str[sizeof...(Cs) + 1] = {transcode_char<T>(Cs)..., T()};
};
} // namespace lexy::_detail

namespace lexy::_detail
{
template <std::size_t N, typename CharT>
struct string_literal
{
    CharT data[N];

    using char_type = CharT;

    consteval string_literal(const CharT* str) : data{}
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

template <template <typename C, C... Cs> typename T, string_literal Str, std::size_t... Idx>
auto _to_type_string(std::index_sequence<Idx...>)
{
    return T<typename decltype(Str)::char_type, Str.data[Idx]...>{};
}
template <template <typename C, C... Cs> typename T, string_literal Str>
using to_type_string
    = decltype(_to_type_string<T, Str>(std::make_index_sequence<decltype(Str)::size()>{}));
} // namespace lexy::_detail

#    define LEXY_NTTP_STRING(T, Str)                                                               \
        ::lexy::_detail::to_type_string<T, ::lexy::_detail::string_literal(Str)>

#endif // LEXY_DETAIL_NTTP_STRING_HPP_INCLUDED

