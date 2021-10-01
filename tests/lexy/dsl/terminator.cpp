// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/terminator.hpp>

#include "verify.hpp"
#include <lexy/dsl/list.hpp>
#include <lexy/dsl/option.hpp>

TEST_CASE("dsl::terminator")
{
    constexpr auto terminator = lexy::dsl::terminator(LEXY_LIT(";"));
    constexpr auto inner      = LEXY_LIT("abc");

    struct callback
    {
        const char* str;

        LEXY_VERIFY_FN auto list()
        {
            struct b
            {
                using return_type = int;

                LEXY_VERIFY_FN void operator()() {}

                LEXY_VERIFY_FN int finish() &&
                {
                    return 42;
                }
            };
            return b{};
        }

        LEXY_VERIFY_FN int success(const char* cur)
        {
            return int(cur - str);
        }
        LEXY_VERIFY_FN int success(const char* cur, int list)
        {
            LEXY_VERIFY_CHECK(list == 42);
            return int(cur - str);
        }
        LEXY_VERIFY_FN int success(const char* cur, lexy::nullopt)
        {
            return int(cur - str);
        }

        LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
        {
            if (e.string() == lexy::_detail::string_view("abc"))
                return -1;
            else if (e.string() == lexy::_detail::string_view(","))
                return -3;
            else if (e.string() == lexy::_detail::string_view(";"))
                return -4;
            else
            {
                LEXY_VERIFY_CHECK(false);
                return -42;
            }
        }
        LEXY_VERIFY_FN int error(test_error<lexy::unexpected_trailing_separator>)
        {
            return -2;
        }
    };

    SUBCASE(".limit()")
    {
        constexpr auto rule = terminator.limit(LEXY_LIT("a")).recovery_rule();
        constexpr auto equivalent
            = lexy::dsl::recover(terminator.terminator()).limit(LEXY_LIT("a"));
        CHECK(std::is_same_v<decltype(rule), decltype(equivalent)>);
    }

    SUBCASE("basic")
    {
        static constexpr auto rule       = terminator(inner);
        constexpr auto        equivalent = inner >> LEXY_LIT(";");
        CHECK(std::is_same_v<decltype(rule), decltype(equivalent)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto success = LEXY_VERIFY("abc;");
        CHECK(success == 4);

        auto no_terminator = LEXY_VERIFY("abc");
        CHECK(no_terminator == -4);
    }
    SUBCASE("try_")
    {
        static constexpr auto rule = terminator.try_(inner);

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto zero = LEXY_VERIFY(";");
        CHECK(zero.value == 1);
        CHECK(zero.errors(-1));
        auto one = LEXY_VERIFY("abc;");
        CHECK(one == 4);

        auto partial = LEXY_VERIFY("ab;");
        CHECK(partial.value == 3);
        CHECK(partial.errors(-1));
        auto invalid = LEXY_VERIFY("abdef;");
        CHECK(invalid.value == 6);
        CHECK(invalid.errors(-1));

        auto no_terminator = LEXY_VERIFY("abc");
        CHECK(no_terminator == -4);
    }
    SUBCASE("opt")
    {
        static constexpr auto rule = terminator.opt(inner);

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto zero = LEXY_VERIFY(";");
        CHECK(zero == 1);
        auto one = LEXY_VERIFY("abc;");
        CHECK(one == 4);

        auto partial = LEXY_VERIFY("ab;");
        CHECK(partial.value == 3);
        CHECK(partial.errors(-1));
        auto invalid = LEXY_VERIFY("abdef;");
        CHECK(invalid.value == 6);
        CHECK(invalid.errors(-1));

        auto no_terminator = LEXY_VERIFY("abc");
        CHECK(no_terminator == -4);
    }
    SUBCASE("list - no sep")
    {
        static constexpr auto rule = terminator.list(inner);

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto zero = LEXY_VERIFY(";");
        CHECK(zero == -1);
        auto one = LEXY_VERIFY("abc;");
        CHECK(one == 4);
        auto two = LEXY_VERIFY("abcabc;");
        CHECK(two == 7);

        auto recover_terminator = LEXY_VERIFY("abcab-;");
        CHECK(recover_terminator.value == 7);
        CHECK(recover_terminator.errors(-1));
        auto recover_item = LEXY_VERIFY("abcab-abc;");
        CHECK(recover_item.value == 10);
        CHECK(recover_item.errors(-1));

        auto no_terminator = LEXY_VERIFY("abc");
        CHECK(no_terminator == -1);
    }
    SUBCASE("list - sep")
    {
        static constexpr auto rule = terminator.list(inner, lexy::dsl::sep(LEXY_LIT(",")));

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto zero = LEXY_VERIFY(";");
        CHECK(zero == -1);
        auto one = LEXY_VERIFY("abc;");
        CHECK(one == 4);
        auto two = LEXY_VERIFY("abc,abc;");
        CHECK(two == 8);

        auto trailing = LEXY_VERIFY("abc,abc,;");
        CHECK(trailing.value == 9);
        CHECK(trailing.errors(-2));

        auto recover_terminator = LEXY_VERIFY("abc,ab-;");
        CHECK(recover_terminator.value == 8);
        CHECK(recover_terminator.errors(-1));
        auto recover_separator = LEXY_VERIFY("abc,ab-,abc;");
        CHECK(recover_separator.value == 12);
        CHECK(recover_separator.errors(-1));

        auto missing_sep = LEXY_VERIFY("abcabc;");
        CHECK(missing_sep.value == 7);
        CHECK(missing_sep.errors(-3));
        auto invalid_sep = LEXY_VERIFY("abc'abc;");
        CHECK(invalid_sep.value == 8);
        CHECK(invalid_sep.errors(-3));

        auto no_terminator = LEXY_VERIFY("abc");
        CHECK(no_terminator == -3);
    }
    SUBCASE("list - trailing sep")
    {
        static constexpr auto rule = terminator.list(inner, lexy::dsl::trailing_sep(LEXY_LIT(",")));

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto zero = LEXY_VERIFY(";");
        CHECK(zero == -1);
        auto one = LEXY_VERIFY("abc;");
        CHECK(one == 4);
        auto two = LEXY_VERIFY("abc,abc;");
        CHECK(two == 8);

        auto trailing = LEXY_VERIFY("abc,abc,;");
        CHECK(trailing == 9);

        auto recover_terminator = LEXY_VERIFY("abc,ab-;");
        CHECK(recover_terminator.value == 8);
        CHECK(recover_terminator.errors(-1));
        auto recover_separator = LEXY_VERIFY("abc,ab-,abc;");
        CHECK(recover_separator.value == 12);
        CHECK(recover_separator.errors(-1));

        auto missing_sep = LEXY_VERIFY("abcabc;");
        CHECK(missing_sep.value == 7);
        CHECK(missing_sep.errors(-3));
        auto invalid_sep = LEXY_VERIFY("abc'abc;");
        CHECK(invalid_sep.value == 8);
        CHECK(invalid_sep.errors(-3));

        auto no_terminator = LEXY_VERIFY("abc");
        CHECK(no_terminator == -3);
    }
    SUBCASE("opt_list - no sep")
    {
        static constexpr auto rule = terminator.opt_list(inner);

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto zero = LEXY_VERIFY(";");
        CHECK(zero == 1);
        auto one = LEXY_VERIFY("abc;");
        CHECK(one == 4);
        auto two = LEXY_VERIFY("abcabc;");
        CHECK(two == 7);

        auto recover_terminator = LEXY_VERIFY("abcab-;");
        CHECK(recover_terminator.value == 7);
        CHECK(recover_terminator.errors(-1));
        auto recover_item = LEXY_VERIFY("abcab-abc;");
        CHECK(recover_item.value == 10);
        CHECK(recover_item.errors(-1));

        auto no_terminator = LEXY_VERIFY("abc");
        CHECK(no_terminator == -1);
    }
    SUBCASE("opt_list - sep")
    {
        static constexpr auto rule = terminator.opt_list(inner, lexy::dsl::sep(LEXY_LIT(",")));

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto zero = LEXY_VERIFY(";");
        CHECK(zero == 1);
        auto one = LEXY_VERIFY("abc;");
        CHECK(one == 4);
        auto two = LEXY_VERIFY("abc,abc;");
        CHECK(two == 8);

        auto trailing = LEXY_VERIFY("abc,abc,;");
        CHECK(trailing.value == 9);
        CHECK(trailing.errors(-2));

        auto recover_terminator = LEXY_VERIFY("abc,ab-;");
        CHECK(recover_terminator.value == 8);
        CHECK(recover_terminator.errors(-1));
        auto recover_separator = LEXY_VERIFY("abc,ab-,abc;");
        CHECK(recover_separator.value == 12);
        CHECK(recover_separator.errors(-1));

        auto missing_sep = LEXY_VERIFY("abcabc;");
        CHECK(missing_sep.value == 7);
        CHECK(missing_sep.errors(-3));
        auto invalid_sep = LEXY_VERIFY("abc'abc;");
        CHECK(invalid_sep.value == 8);
        CHECK(invalid_sep.errors(-3));

        auto no_terminator = LEXY_VERIFY("abc");
        CHECK(no_terminator == -3);
    }
    SUBCASE("opt_list - trailing sep")
    {
        static constexpr auto rule
            = terminator.opt_list(inner, lexy::dsl::trailing_sep(LEXY_LIT(",")));

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto zero = LEXY_VERIFY(";");
        CHECK(zero == 1);
        auto one = LEXY_VERIFY("abc;");
        CHECK(one == 4);
        auto two = LEXY_VERIFY("abc,abc;");
        CHECK(two == 8);

        auto trailing = LEXY_VERIFY("abc,abc,;");
        CHECK(trailing == 9);

        auto recover_terminator = LEXY_VERIFY("abc,ab-;");
        CHECK(recover_terminator.value == 8);
        CHECK(recover_terminator.errors(-1));
        auto recover_separator = LEXY_VERIFY("abc,ab-,abc;");
        CHECK(recover_separator.value == 12);
        CHECK(recover_separator.errors(-1));

        auto missing_sep = LEXY_VERIFY("abcabc;");
        CHECK(missing_sep.value == 7);
        CHECK(missing_sep.errors(-3));
        auto invalid_sep = LEXY_VERIFY("abc'abc;");
        CHECK(invalid_sep.value == 8);
        CHECK(invalid_sep.errors(-3));

        auto no_terminator = LEXY_VERIFY("abc");
        CHECK(no_terminator == -3);
    }
}

