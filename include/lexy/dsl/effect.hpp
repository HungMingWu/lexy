// Copyright (C) 2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#pragma once

#include <lexy/dsl/base.hpp>

namespace lexyd
{

template <auto Fn>
struct _eff : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        constexpr static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if constexpr (requires { Fn(*context.control_block->parse_state); })
            {
                using return_type = decltype(Fn(*context.control_block->parse_state));
                if constexpr (std::is_void_v<return_type>)
                {
                    Fn(*context.control_block->parse_state);
                    return NextParser::parse(context, reader, std::forward<Args>(args)...);
                }
                else
                {
                    return NextParser::parse(context, reader, std::forward<Args>(args)...,
                                             Fn(*context.control_block->parse_state));
                }
            }
            else
            {
                using return_type = decltype(Fn());
                if constexpr (std::is_void_v<return_type>)
                {
                    Fn();
                    return NextParser::parse(context, reader, std::forward<Args>(args)...);
                }
                else
                {
                    return NextParser::parse(context, reader, std::forward<Args>(args)..., Fn());
                }
            }
        }
    };
};

/// Invokes Fn and produces its value as result.
template <auto Fn>
constexpr auto effect = _eff<Fn>{};
} // namespace lexyd


