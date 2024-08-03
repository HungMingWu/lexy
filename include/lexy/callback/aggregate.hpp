// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_CALLBACK_AGGREGATE_HPP_INCLUDED
#define LEXY_CALLBACK_AGGREGATE_HPP_INCLUDED

#include <lexy/callback/base.hpp>
#include <lexy/dsl/member.hpp>

namespace lexy
{
struct nullopt;

template <typename T>
struct _as_aggregate
{
    using return_type = T;
    static_assert(std::is_aggregate_v<return_type>);

    constexpr T operator()(lexy::nullopt&&) const
    {
        return {};
    }
    constexpr T operator()(T&& result) const
    {
        return std::move(result);
    }

    template <typename Fn, typename Value, typename... Tail>
    constexpr T operator()(lexy::member<Fn>, Value&& value, Tail&&... tail) const
    {
        T result{};
        Fn{}(result, std::forward<Value>(value));
        return (*this)(std::move(result), std::forward<Tail>(tail)...);
    }
    template <typename Fn, typename Value, typename... Tail>
    constexpr T operator()(T&& result, lexy::member<Fn>, Value&& value, Tail&&... tail) const
    {
        Fn{}(result, std::forward<Value>(value));
        return (*this)(std::move(result), std::forward<Tail>(tail)...);
    }

    struct _sink
    {
        T _result{};

        using return_type = T;

        template <typename Fn, typename Value>
        constexpr void operator()(lexy::member<Fn>, Value&& value)
        {
            Fn()(_result, std::forward<Value>(value));
        }

        constexpr auto&& finish() &&
        {
            return std::move(_result);
        }
    };
    constexpr auto sink() const
    {
        return _sink{};
    }
};

/// A callback with sink that creates an aggregate.
template <typename T>
constexpr auto as_aggregate = _as_aggregate<T>{};
} // namespace lexy

#endif // LEXY_CALLBACK_AGGREGATE_HPP_INCLUDED

