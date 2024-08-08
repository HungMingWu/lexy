// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_CALLBACK_FOLD_HPP_INCLUDED
#define LEXY_CALLBACK_FOLD_HPP_INCLUDED

#include <lexy/callback/base.hpp>

namespace lexy
{

template <typename T, typename Arg, bool Inplace, typename Op>
struct _fold
{
    Arg                  _init;
    LEXY_EMPTY_MEMBER Op _op;

    struct _sink_callback
    {
        T  _result;
        Op _op;

        using return_type = T;

        template <typename... Args>
        constexpr decltype(auto) operator()(Args&&... args)
        {
            if constexpr (Inplace)
                std::invoke(_op, _result, std::forward<Args>(args)...);
            else
                _result = std::invoke(_op, std::move(_result), std::forward<Args>(args)...);
        }

        constexpr T finish() &&
        {
            return std::move(_result);
        }
    };

    constexpr auto sink() const
    {
        if constexpr (std::is_constructible_v<T, Arg>)
            return _sink_callback{T(_init), _op};
        else
            return _sink_callback{_init(), _op};
    }
};

/// Sink that folds all the arguments with the binary operation op.
template <typename T, typename Arg = T, typename... Op>
constexpr auto fold(Arg&& init, Op&&... op)
{
    auto fn = _make_overloaded(std::forward<Op>(op)...);
    return _fold<T, std::decay_t<Arg>, false, decltype(fn)>{std::forward<Arg>(init), std::move(fn)};
}

/// Sink that folds all the arguments with the binary operation op that modifies the
/// result in-place.
template <typename T, typename Arg = T, typename... Op>
constexpr auto fold_inplace(Arg&& init, Op&&... op)
{
    auto fn = _make_overloaded(std::forward<Op>(op)...);
    return _fold<T, std::decay_t<Arg>, true, decltype(fn)>{std::forward<Arg>(init), std::move(fn)};
}
} // namespace lexy

namespace lexy
{
/// Sink that counts all arguments.
constexpr auto count
    = fold_inplace<std::size_t>(0u, [](std::size_t& result, auto&&...) { ++result; });
} // namespace lexy

#endif // LEXY_CALLBACK_FOLD_HPP_INCLUDED

