// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#pragma once

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>
#include <lexy/dsl/recover.hpp>

namespace lexy
{
// An optional type is something that has the following:
// * a default constructor: this means we can actually construct it from our `nullopt`
// * a dereference operator: this means that it actually contains something else
// * a contextual conversion to bool: this means that it might be "false" (i.e. empty)
//
// This definition should work:
// * it excludes all default constructible types that are convertible to bool (e.g. integers...)
// * it includes pointers, which is ok
// * it includes `std::optional` and all non-std implementations of it
template <typename T>
concept is_optional_like = requires {
    T();
    *std::declval<T&>();
    !std::declval<const T&>();
};

struct nullopt
{
    template <is_optional_like T>
    constexpr operator T() const
    {
        return T();
    }
};
} // namespace lexy

namespace lexyd
{
struct _nullopt : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        constexpr static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            return NextParser::parse(context, reader, std::forward<Args>(args)..., lexy::nullopt{});
        }
    };
};

constexpr auto nullopt = _nullopt{};
} // namespace lexyd

namespace lexyd
{
template <typename Branch>
struct _opt : rule_base
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
                // We don't take the branch and produce a nullopt.
                branch.cancel(context);
                return NextParser::parse(context, reader, std::forward<Args>(args)..., lexy::nullopt{});
            }
        }
    };
};

/// Matches the rule or nothing.
/// In the latter case, produces a `nullopt` value.
template <typename Rule>
constexpr auto opt(Rule)
{
    LEXY_REQUIRE_BRANCH_RULE(Rule, "opt()");
    if constexpr (lexy::is_unconditional_branch_rule<Rule>)
        // Branch is always taken, so don't wrap in opt().
        return Rule{};
    else
        return _opt<Rule>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Term, typename Rule>
struct _optt : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        constexpr static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Try to parse the terminator.
            lexy::branch_parser_for<Term, Reader> term{};
            if (term.try_parse(context.control_block, reader))
                // We had the terminator, so produce nullopt.
                return term.template finish<NextParser>(context, reader, std::forward<Args>(args)...,
                                                        lexy::nullopt{});
            term.cancel(context);

            // We didn't have the terminator, so we parse the rule.
            using parser = lexy::parser_for<Rule, NextParser>;
            return parser::parse(context, reader, std::forward<Args>(args)...);
        }
    };
};
} // namespace lexyd


