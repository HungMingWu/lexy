// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_CALLBACK_OBJECT_HPP_INCLUDED
#define LEXY_CALLBACK_OBJECT_HPP_INCLUDED

#include <lexy/callback/base.hpp>

namespace lexy::_detail
{
template <typename T, typename... Args>
using _detect_brace_construct = decltype(T{std::declval<Args>()...});
template <typename T, typename... Args>
constexpr auto is_brace_constructible = _detail::is_detected<_detect_brace_construct, T, Args...>;

template <typename T, typename... Args>
constexpr auto is_constructible
    = std::is_constructible_v<T, Args...> || is_brace_constructible<T, Args...>;
} // namespace lexy::_detail

namespace lexy
{
template <typename T>
struct _construct
{
    using return_type = T;

    constexpr T operator()(T&& t) const
    {
        return std::move(t);
    }
    constexpr T operator()(const T& t) const
    {
        return t;
    }

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const
        -> std::enable_if_t<_detail::is_constructible<T, Args&&...>, T>
    {
        if constexpr (std::is_constructible_v<T, Args&&...>)
            return T(std::forward<Args>(args)...);
        else
            return T{std::forward<Args>(args)...};
    }
};
template <>
struct _construct<void>
{
    using return_type = void;

    constexpr void operator()() const {}
};

/// A callback that constructs an object of type T by forwarding the arguments.
template <typename T>
constexpr auto construct = _construct<T>{};

template <typename T, typename PtrT>
struct _new
{
    using return_type = PtrT;

    constexpr PtrT operator()(T&& t) const
    {
        auto ptr = new T(std::move(t));
        return PtrT(ptr);
    }
    constexpr PtrT operator()(const T& t) const
    {
        auto ptr = new T(t);
        return PtrT(ptr);
    }

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const
        -> std::enable_if_t<_detail::is_constructible<T, Args&&...>, PtrT>
    {
        if constexpr (std::is_constructible_v<T, Args&&...>)
        {
            auto ptr = new T(std::forward<Args>(args)...);
            return PtrT(ptr);
        }
        else
        {
            auto ptr = new T{std::forward<Args>(args)...};
            return PtrT(ptr);
        }
    }
};

/// A callback that constructs an object of type T on the heap by forwarding the arguments.
template <typename T, typename PtrT = T*>
constexpr auto new_ = _new<T, PtrT>{};
} // namespace lexy

#endif // LEXY_CALLBACK_OBJECT_HPP_INCLUDED

