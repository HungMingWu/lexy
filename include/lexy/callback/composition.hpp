// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_CALLBACK_COMPOSITION_HPP_INCLUDED
#define LEXY_CALLBACK_COMPOSITION_HPP_INCLUDED

#include <lexy/callback/base.hpp>

namespace lexy
{
template <typename Cb, typename State>
struct _compose_state
{
    const Cb& _cb;
    State&    _state;

    using return_type = typename Cb::return_type;

    template <typename... Args>
    constexpr decltype(auto) operator()(Args&&... args) const
    {
        if constexpr (lexy::is_callback_state<Cb, State>) {
	    return _cb[_state](std::forward<Args>(args)...);
        } else {
            return _cb(std::forward<Args>(args)...);
        }
    }
};

template <typename First, typename Second>
struct _compose_cb
{
    LEXY_EMPTY_MEMBER First  _first;
    LEXY_EMPTY_MEMBER Second _second;

    constexpr explicit _compose_cb(First&& first, Second&& second)
    : _first(std::move(first)), _second(std::move(second))
    {}

    using return_type = typename Second::return_type;

    template <typename State>
    requires lexy::is_callback_state<First, State> || lexy::is_callback_state<Second, State>
    constexpr auto operator[](State& state) const
    {
        auto first  = _compose_state<First, State>{_first, state};
        auto second = _compose_state<Second, State>{_second, state};
        return lexy::_compose_cb(std::move(first), std::move(second));
    }

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const
        -> LEXY_DECAY_DECLTYPE(_first(std::forward<Args>(args)...), std::declval<return_type>())
    {
        return _second(_first(std::forward<Args>(args)...));
    }
};

template <typename Sink, typename Callback>
struct _compose_s
{
    LEXY_EMPTY_MEMBER Sink     _sink;
    LEXY_EMPTY_MEMBER Callback _callback;

    using return_type = typename Callback::return_type;

    template <typename... Args>
    constexpr auto sink(Args&&... args) const -> decltype(_sink.sink(std::forward<Args>(args)...))
    {
        return _sink.sink(std::forward<Args>(args)...);
    }

    template <typename State>
    requires lexy::is_callback_state<Callback, State>
    constexpr auto operator[](State& state) const
    {
        return _compose_state<Callback, State>{_callback, state};
    }

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const -> decltype(_callback(std::forward<Args>(args)...))
    {
        return _callback(std::forward<Args>(args)...);
    }
};

/// Composes two callbacks.
template <is_callback First, is_callback Second>
constexpr auto operator|(First first, Second second)
{
    return _compose_cb(std::move(first), std::move(second));
}
template <typename S, typename Cb, typename Second>
constexpr auto operator|(_compose_s<S, Cb> composed, Second second)
{
    auto cb = std::move(composed._callback) | std::move(second);
    return _compose_s<S, decltype(cb)>{std::move(composed._sink), std::move(cb)};
}

/// Composes a sink with a callback.
template <typename Sink, is_callback Callback>
constexpr auto operator>>(Sink sink, Callback cb)
{
    return _compose_s<Sink, Callback>{std::move(sink), std::move(cb)};
}
} // namespace lexy

#endif // LEXY_CALLBACK_COMPOSITION_HPP_INCLUDED

