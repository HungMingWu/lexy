// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/brackets.hpp>

#include "verify.hpp"
#include <lexy/dsl/list.hpp>
#include <lexy/dsl/option.hpp>

TEST_CASE("dsl::brackets()")
{
    constexpr auto brackets = dsl::brackets(dsl::lit_c<'('>, dsl::lit_c<')'>);

    CHECK(equivalent_rules(brackets.open(), dsl::lit_c<'('>));
    CHECK(equivalent_rules(brackets.close(), dsl::lit_c<')'>));
    CHECK(equivalent_rules(brackets.as_terminator(), dsl::terminator(dsl::lit_c<')'>)));
    CHECK(equivalent_rules(brackets.recovery_rule(), dsl::recover(dsl::lit_c<')'>)));

    CHECK(equivalent_rules(brackets(dsl::lit<"abc">),
                           brackets.open() >> brackets.as_terminator()(dsl::lit<"abc">)));
    CHECK(equivalent_rules(brackets.try_(dsl::lit<"abc">),
                           brackets.open() >> brackets.as_terminator().try_(dsl::lit<"abc">)));
    CHECK(equivalent_rules(brackets.opt(dsl::lit<"abc">),
                           brackets.open() >> brackets.as_terminator().opt(dsl::lit<"abc">)));
    CHECK(equivalent_rules(brackets.list(dsl::lit<"abc">),
                           brackets.open() >> brackets.as_terminator().list(dsl::lit<"abc">)));
    CHECK(equivalent_rules(brackets.list(dsl::lit<"abc">, dsl::sep(dsl::lit<",">)),
                           brackets.open()
                               >> brackets.as_terminator().list(dsl::lit<"abc">,
                                                                dsl::sep(dsl::lit<",">))));
    CHECK(equivalent_rules(brackets.opt_list(dsl::lit<"abc">),
                           brackets.open() >> brackets.as_terminator().opt_list(dsl::lit<"abc">)));
    CHECK(equivalent_rules(brackets.opt_list(dsl::lit<"abc">, dsl::sep(dsl::lit<",">)),
                           brackets.open()
                               >> brackets.as_terminator().opt_list(dsl::lit<"abc">,
                                                                    dsl::sep(dsl::lit<",">))));

    CHECK(equivalent_rules(brackets.limit(dsl::lit_c<'!'>).recovery_rule(),
                           dsl::recover(dsl::lit_c<')'>).limit(dsl::lit_c<'!'>)));
    CHECK(equivalent_rules(brackets.limit(dsl::lit_c<'!'>).limit(dsl::lit_c<'.'>),
                           brackets.limit(dsl::lit_c<'!'>, dsl::lit_c<'.'>)));

    CHECK(equivalent_rules(dsl::round_bracketed, brackets));
    CHECK(equivalent_rules(dsl::square_bracketed, dsl::brackets(dsl::lit_c<'['>, dsl::lit_c<']'>)));
    CHECK(equivalent_rules(dsl::curly_bracketed, dsl::brackets(dsl::lit_c<'{'>, dsl::lit_c<'}'>)));
    CHECK(equivalent_rules(dsl::angle_bracketed, dsl::brackets(dsl::lit_c<'<'>, dsl::lit_c<'>'>)));
    CHECK(equivalent_rules(dsl::parenthesized, brackets));
}

