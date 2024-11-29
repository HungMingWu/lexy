#pragma once

#include <cstdint>
#include <utility>

namespace lexy::_detail
{

template <std::size_t... Idx>
constexpr auto _make_index_sequence_tuple(std::index_sequence<Idx...>)
{
    return std::make_tuple(std::integral_constant<std::size_t, Idx>()...);
}

template <std::size_t N>
constexpr auto make_index_sequence_tuple()
{
    return _make_index_sequence_tuple(std::make_index_sequence<N>{});
}

}

