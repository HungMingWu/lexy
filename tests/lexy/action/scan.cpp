// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/action/scan.hpp>

#include <doctest/doctest.h>
#include <lexy/callback/adapter.hpp>
#include <lexy/callback/fold.hpp>
#include <lexy/dsl/capture.hpp>
#include <lexy/dsl/eof.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/peek.hpp>
#include <lexy/dsl/position.hpp>
#include <lexy/dsl/production.hpp>
#include <lexy/dsl/sequence.hpp>
#include <lexy/dsl/whitespace.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy_ext/report_error.hpp>

namespace
{
struct production
{
    static constexpr auto rule = lexy::dsl::capture(lexy::dsl::lit<"abc">);
    static constexpr auto value
        = lexy::callback<int>([](auto lex) { return static_cast<int>(lex.size()); });
};

struct token_production : lexy::token_production
{
    static constexpr auto rule = lexy::dsl::lit<"abc">;
};

struct control_production
{
    static constexpr auto whitespace = lexy::dsl::lit<" ">;
};
} // namespace

TEST_CASE("lexy::scan")
{
    auto check_position = [](const auto& scanner, bool eof, auto pos) {
        CHECK(scanner.is_at_eof() == eof);
        CHECK(scanner.position() == pos);
        CHECK(scanner.remaining_input().reader().position() == pos);
    };

    // We use lexy_ext::report_error to test that error reporting doesn't crash; there was a bug
    // before.
    auto errors = lexy_ext::report_error.to([] {
        struct iterator
        {
            iterator& operator*() noexcept
            {
                return *this;
            }
            iterator& operator++(int) noexcept
            {
                return *this;
            }

            iterator& operator=(char)
            {
                return *this;
            }
        };
        return iterator{};
    }());

    SUBCASE("empty input")
    {
        auto input   = lexy::string_input<lexy::default_encoding>();
        auto scanner = lexy::scan(input, errors);
        CHECK(scanner);
        check_position(scanner, true, input.data());

        scanner.parse(lexy::dsl::eof);
        CHECK(scanner);
        check_position(scanner, true, input.data());

        scanner.parse(lexy::dsl::lit<"abc">);
        CHECK(!scanner);
        check_position(scanner, true, input.data());
    }

    SUBCASE("parse w/o value")
    {
        auto input   = lexy::zstring_input("abc");
        auto scanner = lexy::scan(input, errors);
        CHECK(scanner);
        check_position(scanner, false, input.data());

        scanner.parse(lexy::dsl::lit<"abc">);
        CHECK(scanner);
        check_position(scanner, true, input.data() + 3);

        scanner.parse(lexy::dsl::eof);
        CHECK(scanner);
        check_position(scanner, true, input.data() + 3);

        scanner.parse(lexy::dsl::lit<"abc">);
        CHECK(!scanner);
        check_position(scanner, true, input.data() + 3);
    }
    SUBCASE("parse with value")
    {
        auto input   = lexy::zstring_input("abc");
        auto scanner = lexy::scan(input, errors);
        CHECK(scanner);
        check_position(scanner, false, input.data());

        auto first = scanner.parse<lexy::string_lexeme<>>(lexy::dsl::capture(lexy::dsl::lit<"abc">));
        CHECK(scanner);
        check_position(scanner, true, input.data() + 3);
        CHECK(first);
        CHECK(first.value().begin() == input.data());
        CHECK(first.value().end() == input.data() + 3);

        auto second = scanner.parse<lexy::string_lexeme<>>(lexy::dsl::capture(lexy::dsl::lit<"abc">));
        CHECK(!scanner);
        check_position(scanner, true, input.data() + 3);
        CHECK(!second);
    }
    SUBCASE("parse production")
    {
        auto input   = lexy::zstring_input("abc");
        auto scanner = lexy::scan(input, errors);
        CHECK(scanner);
        check_position(scanner, false, input.data());

        auto first = scanner.parse(production{});
        CHECK(scanner);
        check_position(scanner, true, input.data() + 3);
        CHECK(first);
        CHECK(first.value() == 3);

        auto second = scanner.parse<production>();
        CHECK(!scanner);
        check_position(scanner, true, input.data() + 3);
        CHECK(!second);
    }

    SUBCASE("branch w/o value")
    {
        auto input   = lexy::zstring_input("abcdefa");
        auto scanner = lexy::scan(input, errors);
        CHECK(scanner);
        check_position(scanner, false, input.data());

        auto first = scanner.branch(lexy::dsl::lit<"a"> >> lexy::dsl::lit<"bc">);
        CHECK(scanner);
        check_position(scanner, false, input.data() + 3);
        CHECK(first);

        auto second = scanner.branch(lexy::dsl::lit<"a"> >> lexy::dsl::lit<"bc">);
        CHECK(scanner);
        check_position(scanner, false, input.data() + 3);
        CHECK(!second);

        auto third = scanner.branch(lexy::dsl::lit<"d"> >> lexy::dsl::lit<"ef">);
        CHECK(scanner);
        check_position(scanner, false, input.data() + 6);
        CHECK(third);

        auto fourth = scanner.branch(lexy::dsl::lit<"a"> >> lexy::dsl::lit<"bc">);
        CHECK(!scanner);
        check_position(scanner, true, input.data() + 7);
        CHECK(fourth);
    }
    SUBCASE("branch with value")
    {
        auto input   = lexy::zstring_input("abcdefa");
        auto scanner = lexy::scan(input, errors);
        CHECK(scanner);
        check_position(scanner, false, input.data());

        {
            lexy::scan_result<const char*> result;

            auto taken
                = scanner.branch(result, lexy::dsl::lit<"a"> >> lexy::dsl::position + lexy::dsl::lit<"bc">);
            CHECK(scanner);
            check_position(scanner, false, input.data() + 3);
            CHECK(taken);
            CHECK(result);
            CHECK(result.value() == input.data() + 1);
        }
        {
            lexy::scan_result<const char*> result;

            auto taken
                = scanner.branch(result, lexy::dsl::lit<"a"> >> lexy::dsl::position + lexy::dsl::lit<"bc">);
            CHECK(scanner);
            check_position(scanner, false, input.data() + 3);
            CHECK(!taken);
            CHECK(!result);
        }
        {
            lexy::scan_result<const char*> result;

            auto taken
                = scanner.branch(result, lexy::dsl::lit<"d"> >> lexy::dsl::position + lexy::dsl::lit<"ef">);
            CHECK(scanner);
            check_position(scanner, false, input.data() + 6);
            CHECK(taken);
            CHECK(result);
            CHECK(result.value() == input.data() + 4);
        }
        {
            lexy::scan_result<const char*> result;

            auto taken
                = scanner.branch(result, lexy::dsl::lit<"a"> >> lexy::dsl::position + lexy::dsl::lit<"bc">);
            CHECK(!scanner);
            check_position(scanner, true, input.data() + 7);
            CHECK(taken);
            CHECK(!result);
        }
    }
    SUBCASE("branch production")
    {
        auto input   = lexy::zstring_input("abc");
        auto scanner = lexy::scan(input, errors);
        CHECK(scanner);
        check_position(scanner, false, input.data());

        {
            lexy::scan_result<int> result;

            auto taken = scanner.branch(result, lexy::dsl::p<production>);
            CHECK(scanner);
            check_position(scanner, true, input.data() + 3);
            CHECK(taken);
            CHECK(result);
            CHECK(result.value() == 3);
        }
        {
            lexy::scan_result<int> result;

            auto taken = scanner.branch<production>(result);
            CHECK(scanner);
            check_position(scanner, true, input.data() + 3);
            CHECK(!taken);
            CHECK(!result);
        }
    }

    SUBCASE("error recovery")
    {
        auto input   = lexy::zstring_input("123-abc");
        auto scanner = lexy::scan(input, errors);
        scanner.parse(lexy::dsl::lit<"abc">);
        CHECK(!scanner);
        check_position(scanner, false, input.data());

        // Parsing is a no-op in failed state.
        scanner.parse(lexy::dsl::lit<"123">);
        CHECK(!scanner);
        check_position(scanner, false, input.data());
        // Branch parsing is a no-op in failed state.
        auto taken = scanner.branch(lexy::dsl::lit<"123">);
        CHECK(!scanner);
        check_position(scanner, false, input.data());
        CHECK(!taken);

        auto recovery = scanner.error_recovery();

        // Parsing does something in recovery.
        scanner.parse(lexy::dsl::lit<"123">);
        CHECK(!scanner);
        check_position(scanner, false, input.data() + 3);
        // Branch parsing does something in recovery.
        taken = scanner.branch(lexy::dsl::lit<"-">);
        CHECK(!scanner);
        check_position(scanner, false, input.data() + 4);
        CHECK(taken);

        SUBCASE("finish")
        {
            std::move(recovery).finish();
            CHECK(scanner);
            check_position(scanner, false, input.data() + 4);

            scanner.parse(lexy::dsl::lit<"abc">);
            CHECK(scanner);
            check_position(scanner, true, input.data() + 7);
        }
        SUBCASE("cancel")
        {
            std::move(recovery).cancel();
            CHECK(!scanner);
            check_position(scanner, false, input.data() + 4);

            scanner.parse(lexy::dsl::lit<"abc">);
            CHECK(!scanner);
            check_position(scanner, false, input.data() + 4);

            auto taken = scanner.branch(lexy::dsl::lit<"abc">);
            CHECK(!scanner);
            check_position(scanner, false, input.data() + 4);
            CHECK(!taken);
        }
    }
    SUBCASE("discard")
    {
        auto input   = lexy::zstring_input("abc");
        auto scanner = lexy::scan(input, errors);
        CHECK(scanner);
        check_position(scanner, false, input.data());

        auto result = scanner.discard(lexy::dsl::lit<"abcd">);
        CHECK(scanner);
        check_position(scanner, false, input.data());
        CHECK(!result);

        result = scanner.discard(lexy::dsl::lit<"abc">);
        CHECK(scanner);
        check_position(scanner, true, input.data() + 3);
        CHECK(result);

        result = scanner.discard(lexy::dsl::lit<"abc">);
        CHECK(scanner);
        check_position(scanner, true, input.data() + 3);
        CHECK(!result);
    }
    SUBCASE("error")
    {
        auto input   = lexy::zstring_input("abc");
        auto scanner = lexy::scan(input, lexy::count);
        CHECK(scanner);
        check_position(scanner, false, input.data());

        scanner.error(lexy::expected_char_class{}, scanner.position(), "foo");
        CHECK(scanner);
        check_position(scanner, false, input.data());

        scanner.parse(lexy::dsl::lit<"123">);
        CHECK(!scanner);
        check_position(scanner, false, input.data());

        scanner.error(lexy::expected_char_class{}, scanner.position(), "foo");
        CHECK(!scanner);
        check_position(scanner, false, input.data());

        auto result = std::move(scanner).finish();
        CHECK(result.error_count() == 3);
    }
    SUBCASE("fatal error")
    {
        auto input   = lexy::zstring_input("abc");
        auto scanner = lexy::scan(input, lexy::count);
        CHECK(scanner);
        check_position(scanner, false, input.data());

        scanner.fatal_error(lexy::expected_char_class{}, scanner.position(), "foo");
        CHECK(!scanner);
        check_position(scanner, false, input.data());

        auto result = std::move(scanner).finish();
        CHECK(result.error_count() == 1);
    }

    SUBCASE("peek")
    {
        auto input   = lexy::zstring_input("abc");
        auto scanner = lexy::scan(input, errors);
        CHECK(scanner);
        check_position(scanner, false, input.data());

        auto result = scanner.peek(lexy::dsl::lit<"abc">);
        CHECK(scanner);
        check_position(scanner, false, input.data());
        CHECK(result);

        result = scanner.peek(lexy::dsl::lit<"123">);
        CHECK(scanner);
        check_position(scanner, false, input.data());
        CHECK(!result);
    }

    SUBCASE("control production")
    {
        auto input   = lexy::zstring_input("abc abc");
        auto scanner = lexy::scan<control_production>(input, errors);
        CHECK(scanner);
        check_position(scanner, false, input.data());

        scanner.parse(lexy::dsl::lit<"abc">);
        CHECK(scanner);
        check_position(scanner, false, input.data() + 4);

        scanner.parse(lexy::dsl::lit<"abc">);
        CHECK(scanner);
        check_position(scanner, true, input.data() + 7);
    }

    SUBCASE("capture")
    {
        auto input   = lexy::zstring_input("abcabc");
        auto scanner = lexy::scan(input, errors);
        CHECK(scanner);
        check_position(scanner, false, input.data());

        auto lexeme = scanner.capture(lexy::dsl::lit<"abc">);
        CHECK(scanner);
        check_position(scanner, false, input.data() + 3);
        CHECK(lexeme);
        CHECK(lexeme.value().begin() == input.data());
        CHECK(lexeme.value().end() == input.data() + 3);

        lexeme = scanner.capture(lexy::dsl::p<token_production>);
        CHECK(scanner);
        check_position(scanner, true, input.data() + 6);
        CHECK(lexeme);
        CHECK(lexeme.value().begin() == input.data() + 3);
        CHECK(lexeme.value().end() == input.data() + 6);
    }
}

