// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/any.hpp>

#include "verify.hpp"

TEST_CASE("dsl::any")
{
    constexpr auto rule = lexy::dsl::any;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::success);
    CHECK(empty.trace == test_trace().token(""));

    auto abc = LEXY_VERIFY("abc");
    CHECK(abc.status == test_result::success);
    CHECK(abc.trace == test_trace().token("abc"));

    auto invalid_utf8 = LEXY_VERIFY(lexy::utf8_encoding{}, 'a', 'b', 'c', 0x80, '1', '2', '3');
    CHECK(invalid_utf8.status == test_result::success);
    CHECK(invalid_utf8.trace == test_trace().token("abc_\\u????_123"));
}

