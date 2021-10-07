// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/input/argv_input.hpp>

#include "verify.hpp"

TEST_CASE("argv_separator")
{
    SUBCASE("non-argv_input")
    {
        static constexpr auto rule = lexy::dsl::argv_separator;
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_token_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN int success(const char* cur)
            {
                LEXY_VERIFY_CHECK(cur == str);
                return 0;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.character_class()
                                  == lexy::_detail::string_view("argv-separator"));
                return -1;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto non_empty = LEXY_VERIFY("abc");
        CHECK(non_empty == -1);
    }
    SUBCASE("argv_input")
    {
        char program[] = "IGNORED";
        char first[]   = "abc";
        char second[]  = "de";
        char third[]   = "f";

        char* argv[] = {program, first, second, third, nullptr};
        int   argc   = 4;

        lexy::argv_input input(argc, argv);
        auto             reader = input.reader();

        CHECK(!lexy::try_match_token(lexy::dsl::argv_separator, reader));
        CHECK(reader.peek() == 'a');
        reader.bump();

        CHECK(!lexy::try_match_token(lexy::dsl::argv_separator, reader));
        CHECK(reader.peek() == 'b');
        reader.bump();

        CHECK(!lexy::try_match_token(lexy::dsl::argv_separator, reader));
        CHECK(reader.peek() == 'c');
        reader.bump();

        CHECK(lexy::try_match_token(lexy::dsl::argv_separator, reader));
        CHECK(reader.peek() == 'd');
    }
}

