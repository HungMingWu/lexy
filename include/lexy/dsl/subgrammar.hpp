// Copyright (C) 2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#pragma once

#include <lexy/action/base.hpp>
#include <lexy/dsl/base.hpp>

namespace lexy
{
template <typename Production, typename Handler, typename State, typename Reader>
struct _subgrammar;

template <typename Production, typename Action>
using _subgrammar_for = _subgrammar<Production, typename Action::handler, typename Action::state,
                                    lexy::input_reader<typename Action::input>>;
} // namespace lexy

#define LEXY_DECLARE_SUBGRAMMAR(Production)                                                        \
    namespace lexy                                                                                 \
    {                                                                                              \
        template <typename ParseState>                                                             \
        constexpr auto production_has_value_callback<Production, ParseState> = true;               \
                                                                                                   \
        template <typename Handler, typename State, typename Reader>                               \
        struct _subgrammar<Production, Handler, State, Reader>                                     \
        {                                                                                          \
            template <typename T>                                                                  \
            static bool parse(_detail::lazy_init<T>&                                value,         \
                              _detail::parse_context_control_block<Handler, State>* control_block, \
                              Reader&                                               reader);                                                     \
        };                                                                                         \
    }

#define LEXY_DEFINE_SUBGRAMMAR(Production)                                                         \
    template <typename Handler, typename State, typename Reader>                                   \
    template <typename T>                                                                          \
    bool ::lexy::_subgrammar<Production, Handler, State, Reader>::                                 \
        parse(::lexy::_detail::lazy_init<T>&                                value,                 \
              ::lexy::_detail::parse_context_control_block<Handler, State>* control_block,         \
              Reader&                                                       reader)                \
    {                                                                                              \
        lexy::_pc<Handler, State, Production> context(control_block);                              \
        auto                                  success = ::lexy::_do_action(context, reader);       \
        value                                         = std::move(context.value);                   \
        return success;                                                                            \
    }

#define LEXY_INSTANTIATE_SUBGRAMMAR(Production, ...)                                               \
    template bool ::lexy::_subgrammar_for<Production, __VA_ARGS__> /**/                            \
        ::parse<::lexy::_production_value_type<typename __VA_ARGS__::handler,                      \
                                               typename __VA_ARGS__::state, Production>> /**/      \
        (::lexy::_detail::lazy_init<::lexy::_production_value_type<                                \
             typename __VA_ARGS__::handler, typename __VA_ARGS__::state, Production>>&,            \
         ::lexy::_detail::parse_context_control_block<typename __VA_ARGS__::handler,               \
                                                      typename __VA_ARGS__::state>*,               \
         ::lexy::input_reader<typename __VA_ARGS__::input>&);

namespace lexyd
{
template <typename Production, typename T>
struct _subg : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        constexpr static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using control_block_type = std::decay_t<decltype(*context.control_block)>;
            using handler_type       = typename control_block_type::handler_type;
            using state_type         = typename control_block_type::state_type;

            auto vars                   = context.control_block->vars;
            context.control_block->vars = nullptr;

            constexpr auto production_uses_void_callback = std::is_same_v<
                typename handler_type::template value_callback<Production, state_type>,
                lexy::_detail::void_value_callback>;
            using value_type = std::conditional_t<production_uses_void_callback, void, T>;
            lexy::_detail::lazy_init<value_type> value;

            using subgrammar_traits
                = lexy::_subgrammar<Production, handler_type, state_type, Reader>;
            auto rule_result
                = subgrammar_traits::template parse<value_type>(value, context.control_block,
                                                                reader);

            context.control_block->vars = vars;

            if (!rule_result)
                return false;

            if constexpr (std::is_void_v<value_type>)
                return NextParser::parse(context, reader, std::forward<Args>(args)...);
            else
                return NextParser::parse(context, reader, std::forward<Args>(args)..., *std::move(value));
        }
    };
};

/// Parses the entry production of a subgrammar, which may be defined in a different
/// file.
template <typename Production, typename T>
constexpr auto subgrammar = _subg<Production, T>{};
} // namespace lexyd

