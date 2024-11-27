// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_NEWLINE_HPP_INCLUDED
#define LEXY_DSL_NEWLINE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/token.hpp>

namespace lexy
{
struct expected_newline
{
    static consteval auto name()
    {
        return "expected newline";
    }
};
} // namespace lexy

namespace lexyd
{
struct _nl
: std::decay_t<decltype(literal_set(lexy::dsl::lit<"\n">, lexy::dsl::lit<"\r\n">).error<lexy::expected_newline>)>
{};

/// Matches a newline character.
constexpr auto newline = _nl{};
} // namespace lexyd

namespace lexyd
{
struct _eol : branch_base
{
    template <typename Reader>
    struct bp
    {
        static_assert(lexy::is_char_encoding<typename Reader::encoding>);

        constexpr bool try_parse(const void*, Reader reader)
        {
            return reader.peek() == Reader::encoding::eof()
                   || lexy::try_match_token(newline, reader);
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        constexpr bool finish(Context& context, Reader& reader, Args&&... args)
        {
            if (reader.peek() == Reader::encoding::eof())
            {
                auto pos = reader.position();
                context.on(_ev::token{}, lexy::eof_token_kind, pos, pos);
                return NextParser::parse(context, reader, std::forward<Args>(args)...);
            }
            else
            {
                // Note that we're re-doing the parsing for newline,
                // this looks at most at two characters, so it doesn't really matter.
                return lexy::parser_for<_nl, NextParser>::parse(context, reader, std::forward<Args>(args)...);
            }
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        constexpr static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            static_assert(lexy::is_char_encoding<typename Reader::encoding>);
            return bp<Reader>{}.template finish<NextParser>(context, reader, std::forward<Args>(args)...);
        }
    };
};

/// Matches the end of line (EOF or newline).
constexpr auto eol = _eol{};
} // namespace lexyd

#endif // LEXY_DSL_NEWLINE_HPP_INCLUDED

