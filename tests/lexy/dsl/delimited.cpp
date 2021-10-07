// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <lexy/dsl/delimited.hpp>

#include "verify.hpp"
#include <lexy/dsl/any.hpp>
#include <lexy/dsl/ascii.hpp>
#include <lexy/dsl/eof.hpp>
#include <lexy/dsl/list.hpp>
#include <lexy/dsl/option.hpp>

namespace
{
struct ws_production
{
    static constexpr auto whitespace = LEXY_LIT(" ");
};
} // namespace

TEST_CASE("dsl::delimited()")
{
    constexpr auto cp = lexy::dsl::ascii::character;

    SUBCASE("token")
    {
        static constexpr auto rule = delimited(LEXY_LIT("("), LEXY_LIT(")"))(cp);
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN auto list()
            {
                struct b
                {
                    int count = 0;

                    using return_type = int;

                    LEXY_VERIFY_FN void operator()(lexy::lexeme_for<test_input> lex)
                    {
                        count += int(lex.size());
                    }

                    LEXY_VERIFY_FN int finish() &&
                    {
                        return count;
                    }
                };
                return b{};
            }
            LEXY_VERIFY_FN int success(const char* cur, int count)
            {
                LEXY_VERIFY_CHECK(cur - str >= count + 2);
                return count;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.character() == '(');
                LEXY_VERIFY_CHECK(e.position() == str);
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::missing_delimiter> e)
            {
                LEXY_VERIFY_CHECK(e.begin() == str + 1);
                LEXY_VERIFY_CHECK(e.end() == lexy::_detail::string_view(str).end());
                LEXY_VERIFY_CHECK(e.message() == lexy::_detail::string_view("missing delimiter"));
                return -2;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("ASCII"));
                return -3;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto zero = LEXY_VERIFY("()");
        CHECK(zero == 0);
        auto one = LEXY_VERIFY("(a)");
        CHECK(one == 1);
        auto two = LEXY_VERIFY("(ab)");
        CHECK(two == 2);
        auto three = LEXY_VERIFY("(abc)");
        CHECK(three == 3);

        auto unterminated = LEXY_VERIFY("(abc");
        CHECK(unterminated == -2);

        auto invalid_ascii = LEXY_VERIFY("(ab\xF0"
                                         "c)");
        CHECK(invalid_ascii.value == 3);
        CHECK(invalid_ascii.errors(-3));

        auto inner_whitespace = LEXY_VERIFY_PRODUCTION(ws_production, "(  abc)");
        CHECK(inner_whitespace == 5);
    }
    SUBCASE("branch")
    {
        static constexpr auto rule
            = delimited(LEXY_LIT("(") >> label<0>, LEXY_LIT(")") >> label<1>)(cp);
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN auto list()
            {
                struct b
                {
                    int count = 0;

                    using return_type = int;

                    LEXY_VERIFY_FN void operator()(lexy::lexeme_for<test_input> lex)
                    {
                        count += int(lex.size());
                    }

                    LEXY_VERIFY_FN int finish() &&
                    {
                        return count;
                    }
                };
                return b{};
            }
            LEXY_VERIFY_FN int success(const char* cur, int open, int count, int close)
            {
                LEXY_VERIFY_CHECK(open == 0);
                LEXY_VERIFY_CHECK(close == 1);
                LEXY_VERIFY_CHECK(cur - str >= count + 2);
                return count;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.character() == '(');
                LEXY_VERIFY_CHECK(e.position() == str);
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::missing_delimiter> e)
            {
                LEXY_VERIFY_CHECK(e.begin() == str + 1);
                LEXY_VERIFY_CHECK(e.end() == lexy::_detail::string_view(str).end());
                LEXY_VERIFY_CHECK(e.message() == lexy::_detail::string_view("missing delimiter"));
                return -2;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("ASCII"));
                return -3;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto zero = LEXY_VERIFY("()");
        CHECK(zero == 0);
        auto one = LEXY_VERIFY("(a)");
        CHECK(one == 1);
        auto two = LEXY_VERIFY("(ab)");
        CHECK(two == 2);
        auto three = LEXY_VERIFY("(abc)");
        CHECK(three == 3);

        auto unterminated = LEXY_VERIFY("(abc");
        CHECK(unterminated == -2);

        auto invalid_ascii = LEXY_VERIFY("(ab\xFF"
                                         "c)");
        CHECK(invalid_ascii.value == 3);
        CHECK(invalid_ascii.errors(-3));
    }

    SUBCASE(".limit()")
    {
        static constexpr auto rule
            = delimited(LEXY_LIT("("), LEXY_LIT(")")).limit(LEXY_LIT("!"))(cp);
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN auto list()
            {
                struct b
                {
                    int count = 0;

                    using return_type = int;

                    LEXY_VERIFY_FN void operator()(lexy::lexeme_for<test_input> lex)
                    {
                        count += int(lex.size());
                    }

                    LEXY_VERIFY_FN int finish() &&
                    {
                        return count;
                    }
                };
                return b{};
            }
            LEXY_VERIFY_FN int success(const char* cur, int count)
            {
                LEXY_VERIFY_CHECK(cur - str >= count + 2);
                return count;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.character() == '(');
                LEXY_VERIFY_CHECK(e.position() == str);
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::missing_delimiter> e)
            {
                LEXY_VERIFY_CHECK(e.begin() == str + 1);
                LEXY_VERIFY_CHECK(e.message() == lexy::_detail::string_view("missing delimiter"));
                return -2;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("ASCII"));
                return -3;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto zero = LEXY_VERIFY("()");
        CHECK(zero == 0);
        auto one = LEXY_VERIFY("(a)");
        CHECK(one == 1);
        auto two = LEXY_VERIFY("(ab)");
        CHECK(two == 2);
        auto three = LEXY_VERIFY("(abc)");
        CHECK(three == 3);

        auto unterminated = LEXY_VERIFY("(abc");
        CHECK(unterminated == -2);
        auto unterminated_limited = LEXY_VERIFY("(abc!def");
        CHECK(unterminated_limited == -2);

        auto invalid_ascii = LEXY_VERIFY("(ab\xF0"
                                         "c)");
        CHECK(invalid_ascii.value == 3);
        CHECK(invalid_ascii.errors(-3));

        auto inner_whitespace = LEXY_VERIFY_PRODUCTION(ws_production, "(  abc)");
        CHECK(inner_whitespace == 5);
    }
}

TEST_CASE("dsl::delimited with escape")
{
    constexpr auto cp = lexy::dsl::ascii::character;

    SUBCASE("token")
    {
        static constexpr auto rule
            = delimited(LEXY_LIT("("), LEXY_LIT(")"))(cp, lexy::dsl::escape(LEXY_LIT("$"))
                                                              .capture(lexy::dsl::ascii::print));
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN auto list()
            {
                struct b
                {
                    int count = 0;

                    using return_type = int;

                    LEXY_VERIFY_FN void operator()(lexy::lexeme_for<test_input> lex)
                    {
                        count += int(lex.size());
                    }

                    LEXY_VERIFY_FN int finish() &&
                    {
                        return count;
                    }
                };
                return b{};
            }
            LEXY_VERIFY_FN int success(const char* cur, int count)
            {
                LEXY_VERIFY_CHECK(cur[-1] == ')');
                return count;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.character() == '(');
                LEXY_VERIFY_CHECK(e.position() == str);
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::missing_delimiter> e)
            {
                LEXY_VERIFY_CHECK(e.begin() == str + 1);
                LEXY_VERIFY_CHECK(e.end() == lexy::_detail::string_view(str).end());
                LEXY_VERIFY_CHECK(e.message() == lexy::_detail::string_view("missing delimiter"));
                return -2;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("ASCII"));
                return -3;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::invalid_escape_sequence> e)
            {
                LEXY_VERIFY_CHECK(e.message()
                                  == lexy::_detail::string_view("invalid escape sequence"));
                return -4;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto zero = LEXY_VERIFY("()");
        CHECK(zero == 0);
        auto one = LEXY_VERIFY("(a)");
        CHECK(one == 1);
        auto two = LEXY_VERIFY("(ab)");
        CHECK(two == 2);
        auto three = LEXY_VERIFY("(abc)");
        CHECK(three == 3);

        auto unterminated = LEXY_VERIFY("(abc");
        CHECK(unterminated == -2);

        auto invalid_ascii = LEXY_VERIFY("(ab\xF0)");
        CHECK(invalid_ascii.value == 2);
        CHECK(invalid_ascii.errors(-3));

        auto escape = LEXY_VERIFY("(a$bc$))");
        CHECK(escape == 4); // abc)
        auto invalid_escape = LEXY_VERIFY("(a$\n)");
        CHECK(invalid_escape.value == 2); // a\n
        CHECK(invalid_escape.errors(-4));
    }
    SUBCASE("branch")
    {
        static constexpr auto escape_rule
            = lexy::dsl::escape(LEXY_LIT("$")).capture(lexy::dsl::ascii::print);
        static constexpr auto rule
            = delimited(LEXY_LIT("(") >> label<0>, LEXY_LIT(")") >> label<1>)(cp, escape_rule);

        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN auto list()
            {
                struct b
                {
                    int count = 0;

                    using return_type = int;

                    LEXY_VERIFY_FN void operator()(lexy::lexeme_for<test_input> lex)
                    {
                        count += int(lex.size());
                    }

                    LEXY_VERIFY_FN int finish() &&
                    {
                        return count;
                    }
                };
                return b{};
            }
            LEXY_VERIFY_FN int success(const char* cur, int open, int count, int close)
            {
                LEXY_VERIFY_CHECK(open == 0);
                LEXY_VERIFY_CHECK(close == 1);
                LEXY_VERIFY_CHECK(cur[-1] == ')');
                return count;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.character() == '(');
                LEXY_VERIFY_CHECK(e.position() == str);
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::missing_delimiter> e)
            {
                LEXY_VERIFY_CHECK(e.begin() == str + 1);
                LEXY_VERIFY_CHECK(e.end() == lexy::_detail::string_view(str).end());
                LEXY_VERIFY_CHECK(e.message() == lexy::_detail::string_view("missing delimiter"));
                return -2;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("ASCII"));
                return -3;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::invalid_escape_sequence> e)
            {
                LEXY_VERIFY_CHECK(e.message()
                                  == lexy::_detail::string_view("invalid escape sequence"));
                return -4;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto zero = LEXY_VERIFY("()");
        CHECK(zero == 0);
        auto one = LEXY_VERIFY("(a)");
        CHECK(one == 1);
        auto two = LEXY_VERIFY("(ab)");
        CHECK(two == 2);
        auto three = LEXY_VERIFY("(abc)");
        CHECK(three == 3);

        auto unterminated = LEXY_VERIFY("(abc");
        CHECK(unterminated == -2);

        auto invalid_ascii = LEXY_VERIFY("(ab\xF0)");
        CHECK(invalid_ascii.value == 2);
        CHECK(invalid_ascii.errors(-3));

        auto escape = LEXY_VERIFY("(a$bc$))");
        CHECK(escape == 4); // abc)
        auto invalid_escape = LEXY_VERIFY("(a$\n)");
        CHECK(invalid_escape.value == 2); // a\n
        CHECK(invalid_escape.errors(-4));
    }

    SUBCASE(".limit()")
    {
        static constexpr auto delim = delimited(LEXY_LIT("("), LEXY_LIT(")")).limit(LEXY_LIT("!"));
        static constexpr auto rule
            = delim(cp, lexy::dsl::escape(LEXY_LIT("$")).capture(lexy::dsl::ascii::print));
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN auto list()
            {
                struct b
                {
                    int count = 0;

                    using return_type = int;

                    LEXY_VERIFY_FN void operator()(lexy::lexeme_for<test_input> lex)
                    {
                        count += int(lex.size());
                    }

                    LEXY_VERIFY_FN int finish() &&
                    {
                        return count;
                    }
                };
                return b{};
            }
            LEXY_VERIFY_FN int success(const char* cur, int count)
            {
                LEXY_VERIFY_CHECK(cur[-1] == ')');
                return count;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.character() == '(');
                LEXY_VERIFY_CHECK(e.position() == str);
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::missing_delimiter> e)
            {
                LEXY_VERIFY_CHECK(e.begin() == str + 1);
                LEXY_VERIFY_CHECK(e.message() == lexy::_detail::string_view("missing delimiter"));
                return -2;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("ASCII"));
                return -3;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::invalid_escape_sequence> e)
            {
                LEXY_VERIFY_CHECK(e.message()
                                  == lexy::_detail::string_view("invalid escape sequence"));
                return -4;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto zero = LEXY_VERIFY("()");
        CHECK(zero == 0);
        auto one = LEXY_VERIFY("(a)");
        CHECK(one == 1);
        auto two = LEXY_VERIFY("(ab)");
        CHECK(two == 2);
        auto three = LEXY_VERIFY("(abc)");
        CHECK(three == 3);

        auto unterminated = LEXY_VERIFY("(abc");
        CHECK(unterminated == -2);
        auto unterminated_limited = LEXY_VERIFY("(abc!def");
        CHECK(unterminated_limited == -2);

        auto invalid_ascii = LEXY_VERIFY("(ab\xF0)");
        CHECK(invalid_ascii.value == 2);
        CHECK(invalid_ascii.errors(-3));

        auto escape = LEXY_VERIFY("(a$bc$))");
        CHECK(escape == 4); // abc)
        auto invalid_escape = LEXY_VERIFY("(a$\n)");
        CHECK(invalid_escape.value == 2); // a\n
        CHECK(invalid_escape.errors(-4));
    }

    SUBCASE("multiple escapes")
    {
        static constexpr auto rule
            = delimited(LEXY_LIT("("),
                        LEXY_LIT(
                            ")"))(cp,
                                  lexy::dsl::escape(LEXY_LIT("$")).capture(lexy::dsl::ascii::upper),
                                  lexy::dsl::escape(LEXY_LIT("\\"))
                                      .capture(lexy::dsl::ascii::lower));
        CHECK(lexy::is_rule<decltype(rule)>);
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN auto list()
            {
                struct b
                {
                    int count = 0;

                    using return_type = int;

                    LEXY_VERIFY_FN void operator()(lexy::lexeme_for<test_input> lex)
                    {
                        count += int(lex.size());
                    }

                    LEXY_VERIFY_FN int finish() &&
                    {
                        return count;
                    }
                };
                return b{};
            }
            LEXY_VERIFY_FN int success(const char* cur, int count)
            {
                LEXY_VERIFY_CHECK(cur[-1] == ')');
                return count;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.character() == '(');
                LEXY_VERIFY_CHECK(e.position() == str);
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::missing_delimiter> e)
            {
                LEXY_VERIFY_CHECK(e.begin() == str + 1);
                LEXY_VERIFY_CHECK(e.end() == lexy::_detail::string_view(str).end());
                LEXY_VERIFY_CHECK(e.message() == lexy::_detail::string_view("missing delimiter"));
                return -2;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::expected_char_class> e)
            {
                LEXY_VERIFY_CHECK(e.character_class() == lexy::_detail::string_view("ASCII"));
                return -3;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::invalid_escape_sequence> e)
            {
                LEXY_VERIFY_CHECK(e.message()
                                  == lexy::_detail::string_view("invalid escape sequence"));
                return -4;
            }
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty == -1);

        auto zero = LEXY_VERIFY("()");
        CHECK(zero == 0);
        auto one = LEXY_VERIFY("(a)");
        CHECK(one == 1);
        auto two = LEXY_VERIFY("(ab)");
        CHECK(two == 2);
        auto three = LEXY_VERIFY("(abc)");
        CHECK(three == 3);

        auto unterminated = LEXY_VERIFY("(abc");
        CHECK(unterminated == -2);

        auto invalid_ascii = LEXY_VERIFY("(ab\xF0)");
        CHECK(invalid_ascii.value == 2);
        CHECK(invalid_ascii.errors(-3));

        auto escape_one = LEXY_VERIFY("(a$Bc$D)");
        CHECK(escape_one == 4); // abc)
        auto invalid_escape_one = LEXY_VERIFY("(a$b)");
        CHECK(invalid_escape_one.value == 2); // ab
        CHECK(invalid_escape_one.errors(-4));

        auto escape_two = LEXY_VERIFY("(a\\bc\\d)");
        CHECK(escape_two == 4); // abc)
        auto invalid_escape_two = LEXY_VERIFY("(a\\B)");
        CHECK(invalid_escape_two.value == 2); // aB
        CHECK(invalid_escape_two.errors(-4));

        auto both_escapes = LEXY_VERIFY("(a$Bc\\d)");
        CHECK(both_escapes == 4);
    }
}

TEST_CASE("predefined dsl::delimited")
{
    constexpr auto quoted_equivalent = lexy::dsl::delimited(LEXY_LIT("\""));
    CHECK(std::is_same_v<decltype(lexy::dsl::quoted), decltype(quoted_equivalent)>);
    constexpr auto triple_quoted_equivalent = lexy::dsl::delimited(LEXY_LIT("\"\"\""));
    CHECK(std::is_same_v<decltype(lexy::dsl::triple_quoted), decltype(triple_quoted_equivalent)>);
    constexpr auto single_quoted_equivalent = lexy::dsl::delimited(LEXY_LIT("'"));
    CHECK(std::is_same_v<decltype(lexy::dsl::single_quoted), decltype(single_quoted_equivalent)>);

    constexpr auto backticked_equivalent = lexy::dsl::delimited(LEXY_LIT("`"));
    CHECK(std::is_same_v<decltype(lexy::dsl::backticked), decltype(backticked_equivalent)>);
    constexpr auto double_backticked_equivalent = lexy::dsl::delimited(LEXY_LIT("``"));
    CHECK(std::is_same_v<decltype(lexy::dsl::double_backticked),
                         decltype(double_backticked_equivalent)>);
    constexpr auto triple_backticked_equivalent = lexy::dsl::delimited(LEXY_LIT("```"));
    CHECK(std::is_same_v<decltype(lexy::dsl::triple_backticked),
                         decltype(triple_backticked_equivalent)>);
}

namespace
{
constexpr auto symbols = lexy::symbol_table<char>.map<'a'>('a');
}

TEST_CASE("dsl::escape")
{
    constexpr auto escape = lexy::dsl::escape(LEXY_LIT("$"));
    constexpr auto delim  = [](auto e) { return lexy::dsl::single_quoted(lexy::dsl::any, e); };

    SUBCASE(".rule()")
    {
        static constexpr auto rule = delim(escape.rule(LEXY_LIT("abc") >> label<0>));

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN auto list()
            {
                struct b
                {
                    int result = -1;

                    using return_type = int;

                    LEXY_VERIFY_FN void operator()(lexy::lexeme_for<test_input>) {}
                    LEXY_VERIFY_FN void operator()(int i)
                    {
                        result = i;
                    }

                    LEXY_VERIFY_FN int finish() &&
                    {
                        return result;
                    }
                };
                return b{};
            }

            LEXY_VERIFY_FN int success(const char* cur, int i)
            {
                LEXY_VERIFY_CHECK(cur == str + 6);
                return i;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.character() == '$');
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::invalid_escape_sequence> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str + 1);
                return -2;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::missing_delimiter>)
            {
                return -3;
            }
        };

        auto abc = LEXY_VERIFY("'$abc'");
        CHECK(abc == 0);

        auto invalid = LEXY_VERIFY("'$ab'");
        CHECK(invalid.value == -1);
        CHECK(invalid.errors(-2, -3));
    }
    SUBCASE("multiple rules")
    {
        static constexpr auto rule = delim(escape.rule(LEXY_LIT("a") >> label<1>)
                                               .rule(LEXY_LIT("b") >> label<2>)
                                               .rule(lexy::dsl::else_ >> label<0>));

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN auto list()
            {
                struct b
                {
                    int result = -1;

                    using return_type = int;

                    LEXY_VERIFY_FN void operator()(lexy::lexeme_for<test_input>) {}
                    LEXY_VERIFY_FN void operator()(int i)
                    {
                        result = i;
                    }

                    LEXY_VERIFY_FN int finish() &&
                    {
                        return result;
                    }
                };
                return b{};
            }

            LEXY_VERIFY_FN int success(const char*, int i)
            {
                return i;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.character() == '$');
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::missing_delimiter>)
            {
                return -3;
            }
        };

        auto a = LEXY_VERIFY("'$a'");
        CHECK(a == 1);
        auto b = LEXY_VERIFY("'$b'");
        CHECK(b == 2);

        auto invalid = LEXY_VERIFY("'$c'");
        CHECK(invalid == -3);
    }
    SUBCASE(".capture()")
    {
        static constexpr auto rule = delim(escape.capture(lexy::dsl::ascii::character));

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN auto list()
            {
                struct b
                {
                    char result = 0;

                    using return_type = char;

                    LEXY_VERIFY_FN void operator()(lexy::lexeme_for<test_input> lex)
                    {
                        result = *lex.begin();
                    }

                    LEXY_VERIFY_FN auto finish() &&
                    {
                        return result;
                    }
                };
                return b{};
            }

            LEXY_VERIFY_FN int success(const char* cur, char c)
            {
                LEXY_VERIFY_CHECK(cur == str + 4);
                return c;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.character() == '$');
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::invalid_escape_sequence> e)
            {
                LEXY_VERIFY_CHECK(e.begin() == str + 1);
                LEXY_VERIFY_CHECK(e.end() == str + 2);
                return -2;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::missing_delimiter>)
            {
                return -3;
            }
        };

        auto a = LEXY_VERIFY("'$a'");
        CHECK(a == 'a');
        auto b = LEXY_VERIFY("'$b'");
        CHECK(b == 'b');

        auto invalid = LEXY_VERIFY("'$\xFF'");
        CHECK(invalid.value == -1);
        CHECK(invalid.errors(-2, -3));
    }
    SUBCASE(".symbol()")
    {
        static constexpr auto rule = delim(escape.symbol<symbols>());

        struct callback
        {
            const char* str;

            LEXY_VERIFY_FN auto list()
            {
                struct b
                {
                    char result = 0;

                    using return_type = char;

                    LEXY_VERIFY_FN void operator()(lexy::lexeme_for<test_input>) {}
                    LEXY_VERIFY_FN void operator()(char c)
                    {
                        result = c;
                    }

                    LEXY_VERIFY_FN auto finish() &&
                    {
                        return result;
                    }
                };
                return b{};
            }

            LEXY_VERIFY_FN int success(const char* cur, char c)
            {
                LEXY_VERIFY_CHECK(cur == str + 4);
                return c;
            }

            LEXY_VERIFY_FN int error(test_error<lexy::expected_literal> e)
            {
                LEXY_VERIFY_CHECK(e.position() == str);
                LEXY_VERIFY_CHECK(e.character() == '$');
                return -1;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::invalid_escape_sequence> e)
            {
                LEXY_VERIFY_CHECK(e.begin() == str + 1);
                LEXY_VERIFY_CHECK(e.end() == str + 2);
                return -2;
            }
            LEXY_VERIFY_FN int error(test_error<lexy::missing_delimiter>)
            {
                return -3;
            }
        };

        auto a = LEXY_VERIFY("'$a'");
        CHECK(a == 'a');

        auto invalid = LEXY_VERIFY("'$b'");
        CHECK(invalid.value == -1);
        CHECK(invalid.errors(-2, -3));
    }
}

TEST_CASE("predefined escapes")
{
    constexpr auto backslash_equivalent = lexy::dsl::escape(LEXY_LIT("\\"));
    CHECK(std::is_same_v<decltype(lexy::dsl::backslash_escape), decltype(backslash_equivalent)>);

    constexpr auto dollar_equivalent = lexy::dsl::escape(LEXY_LIT("$"));
    CHECK(std::is_same_v<decltype(lexy::dsl::dollar_escape), decltype(dollar_equivalent)>);
}

