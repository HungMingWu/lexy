// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_CALLBACK_BASE_HPP_INCLUDED
#define LEXY_CALLBACK_BASE_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/_detail/detect.hpp>
#include <lexy/_detail/invoke.hpp>

namespace lexy
{
template <typename T>
concept is_callback = requires {
	typename T::return_type;
};

template <typename T, typename... Args>
concept is_callback_for = requires {
    std::declval<T>()(std::declval<Args>()...);
};

template <typename T, typename State>
using _detect_callback_state = decltype(std::declval<const T>()[std::declval<State&>()]);
template <typename T, typename State>
constexpr bool is_callback_state
    = _detail::is_detected<_detect_callback_state, T, std::decay_t<State>>;

template <typename T, typename State, typename... Args>
using _detect_callback_with_state_for
    = decltype(std::declval<const T>()[std::declval<State&>()](std::declval<Args>()...));
template <typename T, typename State, typename... Args>
constexpr bool is_callback_with_state_for
    = _detail::is_detected<_detect_callback_with_state_for, std::decay_t<T>, State, Args...>;

/// Returns the type of the `.sink()` function.
template <typename Sink, typename... Args>
using sink_callback = decltype(std::declval<Sink>().sink(std::declval<Args>()...));

template <typename T, typename... Args>
using _detect_sink_callback_for = decltype(std::declval<T&>()(std::declval<Args>()...));
template <typename T, typename... Args>
constexpr bool is_sink_callback_for
    = _detail::is_detected<_detect_sink_callback_for, std::decay_t<T>, Args...>;

template <typename T, typename... Args>
concept is_sink = requires {
    std::declval<T>().sink(std::declval<Args>()...).finish();
};

} // namespace lexy

namespace lexy
{
template <typename Fn>
struct _fn_holder
{
    Fn fn;

    constexpr explicit _fn_holder(Fn fn) : fn(fn) {}

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const
        -> decltype(_detail::invoke(fn, std::forward<Args>(args)...))
    {
        return _detail::invoke(fn, std::forward<Args>(args)...);
    }
};

template <typename Fn>
using _fn_as_base = std::conditional_t<std::is_class_v<Fn>, Fn, _fn_holder<Fn>>;

template <typename... Fns>
struct _overloaded : _fn_as_base<Fns>...
{
    constexpr explicit _overloaded(Fns... fns) : _fn_as_base<Fns>(std::move(fns))... {}

    using _fn_as_base<Fns>::operator()...;
};

template <typename... Op>
constexpr auto _make_overloaded(Op&&... op)
{
    if constexpr (sizeof...(Op) == 1)
        return (std::forward<Op>(op), ...);
    else
        return _overloaded(std::forward<Op>(op)...);
}
} // namespace lexy

#endif // LEXY_CALLBACK_BASE_HPP_INCLUDED
