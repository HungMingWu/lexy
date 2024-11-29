// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#pragma once

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>

namespace lexyd
{
template <typename Branch>
struct _if : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        constexpr static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            lexy::branch_parser_for<Branch, Reader> branch{};
            if (branch.try_parse(context.control_block, reader))
                // We take the branch.
                return branch.template finish<NextParser>(context, reader, std::forward<Args>(args)...);
            else
            {
                // We don't take the branch.
                branch.cancel(context);
                return NextParser::parse(context, reader, std::forward<Args>(args)...);
            }
        }
    };
};

/// If the branch condition matches, matches the branch then.
template <typename Branch>
constexpr auto if_(Branch)
{
    LEXY_REQUIRE_BRANCH_RULE(Branch, "if()");
    if constexpr (lexy::is_unconditional_branch_rule<Branch>)
        // Branch is always taken, so don't wrap in if_().
        return Branch{};
    else
        return _if<Branch>{};
}
} // namespace lexyd


