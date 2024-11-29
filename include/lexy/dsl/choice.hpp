// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#pragma once

#include <tuple>
#include <lexy/_detail/util.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/error.hpp>

namespace lexy
{
struct exhausted_choice
{
    static consteval auto name()
    {
        return "exhausted choice";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename... R>
struct _chc
// Only make it a branch rule if it doesn't have an unconditional branch.
// A choice rule with an unconditional branch is itself an unconditional branch, which is most
// likely a bug.
: std::conditional_t<(lexy::is_unconditional_branch_rule<R> || ...), rule_base, branch_base>
{
    static constexpr auto _any_unconditional = (lexy::is_unconditional_branch_rule<R> || ...);

    template <typename Reader>
    struct bp
    {
        template <typename Rule>
        using rp = lexy::branch_parser_for<Rule, Reader>;

        std::tuple<rp<R>...> r_parsers;
        std::size_t                    branch_idx;

        template <typename ControlBlock>
        constexpr auto try_parse(const ControlBlock* cb, const Reader& reader)
            -> std::conditional_t<_any_unconditional, std::true_type, bool>
        {
            auto try_r = [&](std::size_t idx, auto& parser) {
                if (!parser.try_parse(cb, reader))
                    return false;

                branch_idx = idx;
                return true;
            };

            // Need to try each possible branch.
            auto found_branch = std::apply([&](auto... Idx) {
                return (try_r(Idx, std::get<Idx>(r_parsers)) || ...);
            }, lexy::_detail::make_index_sequence_tuple<sizeof...(R)>());

            if constexpr (_any_unconditional)
            {
                LEXY_ASSERT(found_branch,
                            "it is unconditional, but we still haven't found a rule?!");
                return {};
            }
            else
            {
                return found_branch;
            }
        }

        template <typename Context>
        constexpr void cancel(Context& context)
        {
            // Need to cancel all branches.
            std::apply([&](auto... Idx) {
                (std::get<Idx>(r_parsers).cancel(context), ...);
            }, lexy::_detail::make_index_sequence_tuple<sizeof...(R)>());
        }

        template <typename NextParser, typename Context, typename... Args>
        constexpr bool finish(Context& context, Reader& reader, Args&&... args)
        {
            // Need to call finish on the selected branch, and cancel on all others before that.
            auto result = false;
            std::apply([&](auto... Idx) {
                (void)((Idx == branch_idx
                        ? (result
                           = std::get<Idx>(r_parsers)
                                 .template finish<NextParser>(context, reader, std::forward<Args>(args)...),
                           true)
                        : (std::get<Idx>(r_parsers).cancel(context), false))
                   || ...);
            }, lexy::_detail::make_index_sequence_tuple<sizeof...(R)>());
            return result;
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        constexpr static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto result = false;
            auto try_r  = [&](auto&& parser) {
                if (!parser.try_parse(context.control_block, reader))
                {
                    parser.cancel(context);
                    return false;
                }

                // std::forward<Args>(args) will break MSVC builds targeting C++17.
                result = parser.template finish<NextParser>(context, reader,
                                                            static_cast<Args&&>(args)...);
                return true;
            };

            // Try to parse each branch in order.
            auto found_branch = (try_r(lexy::branch_parser_for<R, Reader>{}) || ...);
            if constexpr (_any_unconditional)
            {
                LEXY_ASSERT(found_branch,
                            "it is unconditional, but we still haven't found a rule?!");
                return result;
            }
            else
            {
                if (found_branch)
                    return result;

                auto err = lexy::error<Reader, lexy::exhausted_choice>(reader.position());
                context.on(_ev::error{}, err);
                return false;
            }
        }
    };
};

template <typename R, typename S>
constexpr auto operator|(R, S)
{
    LEXY_REQUIRE_BRANCH_RULE(R, "choice");
    LEXY_REQUIRE_BRANCH_RULE(S, "choice");
    return _chc<R, S>{};
}
template <typename... R, typename S>
constexpr auto operator|(_chc<R...>, S)
{
    LEXY_REQUIRE_BRANCH_RULE(S, "choice");
    return _chc<R..., S>{};
}
template <typename R, typename... S>
constexpr auto operator|(R, _chc<S...>)
{
    LEXY_REQUIRE_BRANCH_RULE(R, "choice");
    return _chc<R, S...>{};
}
template <typename... R, typename... S>
constexpr auto operator|(_chc<R...>, _chc<S...>)
{
    return _chc<R..., S...>{};
}
} // namespace lexyd


