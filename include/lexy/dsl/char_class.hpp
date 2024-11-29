// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#pragma once

#include <lexy/_detail/code_point.hpp>
#include <lexy/_detail/nttp_string.hpp>
#include <lexy/_detail/swar.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>

namespace lexy::_detail
{

template <typename T, std::size_t N, typename F>
consteval auto transform(const T (&src)[N], F func)
{
    using dst_type = decltype(func(src[0]));
    std::array<dst_type, N> dst;
    for (size_t i = 0; i < N; i++)
        dst[i] = func(src[i]);
    return dst;
}

struct ascii_set
{
    bool contains[128];

    constexpr ascii_set() : contains{} {}

    template <typename Fn>
    constexpr void visit(Fn fn) const
    {
        for (auto i = 0; i != 128; ++i)
            if (contains[i])
                fn(i);
    }

    template <typename Fn>
    constexpr void visit_range(Fn fn) const
    {
        auto range_begin = -1;
        auto last_char   = range_begin;
        visit([&](int c) {
            if (range_begin == -1)
            {
                range_begin = c;
                last_char   = c;
            }
            else if (last_char + 1 == c)
            {
                last_char = c;
            }
            else
            {
                fn(range_begin, last_char);
                range_begin = c;
                last_char   = c;
            }
        });
        if (range_begin != -1)
            fn(range_begin, last_char);
    }

    constexpr void insert(int c)
    {
        contains[c] = true;
    }
    constexpr void insert(int lower, int upper)
    {
        for (auto i = lower; i <= upper; ++i)
            contains[i] = true;
    }
    constexpr void insert(const ascii_set& other)
    {
        other.visit([&](int c) { contains[c] = true; });
    }
    constexpr void remove(const ascii_set& other)
    {
        other.visit([&](int c) { contains[c] = false; });
    }
};

template <std::size_t RangeCount, std::size_t SingleCount>
struct compressed_ascii_set
{
    char range_lower[RangeCount == 0 ? 1 : RangeCount];
    char range_upper[RangeCount == 0 ? 1 : RangeCount];
    char singles[SingleCount == 0 ? 1 : SingleCount];

    static constexpr std::size_t range_count()
    {
        return RangeCount;
    }
    static constexpr std::size_t single_count()
    {
        return SingleCount;
    }
};

template <typename T>
constexpr auto compress_ascii_set()
{
    constexpr auto set = T::char_class_ascii();

    constexpr auto count = [&set] {
        struct result_t
        {
            std::size_t range_count;
            std::size_t single_count;
        } result{0, 0};

        set.visit_range([&](int lower, int upper) {
            if (lower != upper)
                ++result.range_count;
            else
                ++result.single_count;
        });

        return result;
    }();

    compressed_ascii_set<count.range_count, count.single_count> result{};

    auto cur_range  = 0u;
    auto cur_single = 0u;
    set.visit_range([&](int lower, int upper) {
        if (lower != upper)
        {
            result.range_lower[cur_range] = char(lower);
            result.range_upper[cur_range] = char(upper);
            ++cur_range;
        }
        else
        {
            result.singles[cur_single] = char(lower);
            ++cur_single;
        }
    });

    return result;
}
} // namespace lexy::_detail

namespace lexy::_detail
{
template <const auto& CompressedAsciiSet>
struct ascii_set_matcher
{
    template <typename Encoding>
    static consteval auto to_int_type(char c)
    {
        return Encoding::to_int_type(static_cast<typename Encoding::char_type>(c));
    }

    template <typename Encoding>
    inline static constexpr bool match([[maybe_unused]] typename Encoding::int_type cur)
    {
        // It must be in one of the ranges...
        auto transform_lower = transform(CompressedAsciiSet.range_lower, to_int_type<Encoding>);
        auto transform_upper = transform(CompressedAsciiSet.range_upper, to_int_type<Encoding>);
        for (std::size_t i = 0; i < CompressedAsciiSet.range_count(); i++)
            if (transform_lower[i] <= cur && cur <= transform_upper[i])
                return true;
        auto transform_signles = transform(CompressedAsciiSet.singles, to_int_type<Encoding>);
        // or one of the single characters.
        for (std::size_t i = 0; i < CompressedAsciiSet.single_count(); i++)
            if (cur == transform_signles[i])
                return true;
        return false;
    }
};
} // namespace lexy::_detail

namespace lexyd
{
template <typename CharSet>
constexpr auto _cas = lexy::_detail::compress_ascii_set<CharSet>();

template <typename Derived>
struct char_class_base : token_base<Derived>, _char_class_base
{
    //=== "virtual" functions ===//
    // static const char* char_class_name();
    // static ascii_set char_class_ascii();

    static constexpr bool char_class_unicode()
    {
        return true;
    }

    static constexpr std::false_type char_class_match_cp(char32_t)
    {
        return {};
    }

    template <typename Reader, typename Context>
    static constexpr void char_class_report_error(Context&                  context,
                                                  typename Reader::iterator position)
    {
        constexpr auto name = Derived::char_class_name();
        auto           err  = lexy::error<Reader, lexy::expected_char_class>(position, name);
        context.on(_ev::error{}, err);
    }

    /// Returns true if c contains only characters from the char class.
    /// If it returns false, it may still be valid, it just couldn't be detected.
    template <typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int)
    {
        return std::false_type{};
    }

    //=== provided functions ===//
    template <typename Reader>
    struct tp
    {
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr bool try_parse(Reader reader)
        {
            static_assert(lexy::is_char_encoding<typename Reader::encoding>);

            using matcher = lexy::_detail::ascii_set_matcher<_cas<Derived>>;
            if (matcher::template match<typename Reader::encoding>(reader.peek()))
            {
                reader.bump();
                end = reader.current();
                return true;
            }

            if constexpr (std::is_same_v<decltype(Derived::char_class_match_cp(char32_t())),
                                         std::false_type>)
            {
                return false;
            }
            else if constexpr (lexy::is_unicode_encoding<typename Reader::encoding>)
            {
                static_assert(Derived::char_class_unicode(),
                              "cannot use this character class with Unicode encoding");

                // Parse one code point.
                auto result = lexy::_detail::parse_code_point(reader);
                if (result.error != lexy::_detail::cp_error::success)
                    return false;

                if (!Derived::char_class_match_cp(result.cp))
                    return false;

                end = result.end;
                return true;
            }
            else
            {
                static_assert(!Derived::char_class_unicode(),
                              "cannot use this character class with non-Unicode char encodings");

                if (reader.peek() == Reader::encoding::eof())
                    return false;

                auto cp = static_cast<char32_t>(reader.peek());
                reader.bump();

                if (!Derived::char_class_match_cp(cp))
                    return false;

                end = reader.current();
                return true;
            }
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            Derived::template char_class_report_error<Reader>(context, reader.position());
        }
    };
};
} // namespace lexyd

template <lexy::_detail::string_literal Name>
constexpr auto define_char_class(auto rule) {
    using T = decltype(rule);
    static_assert(::lexy::is_char_class_rule<T>);
    struct c : ::lexyd::char_class_base<c>
    {
        static constexpr auto char_class_unicode()
        {
            return T::char_class_unicode();
        }
        static consteval auto char_class_name()
        {
            return lexy::_detail::type_string<Name>::c_str();
        }
        static consteval auto char_class_ascii()
        {
            return T::char_class_ascii();
        }
        static constexpr auto char_class_match_cp(char32_t cp)
        {
            return T::char_class_match_cp(cp);
        }
    };
    return c{};
}

namespace lexyd
{
template <lexy::_detail::string_literal Str>
struct _lit;

template <lexy::_detail::string_literal Str>
struct _lcp;

// Implementation helper for the literal overloads.
template <char32_t Cp>
struct _ccp : char_class_base<_ccp<Cp>>
{
    static consteval auto char_class_name()
    {
        return "code-point";
    }

    static consteval auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        if constexpr (Cp <= 0x7F)
            result.insert(Cp);
        return result;
    }

    static constexpr auto char_class_match_cp([[maybe_unused]] char32_t cp)
    {
        if constexpr (Cp <= 0x7F)
            return std::false_type{};
        else
            return cp == Cp;
    }
};
template <unsigned char Byte>
struct _cb : char_class_base<_cb<Byte>>
{
    static constexpr auto char_class_unicode()
    {
        return Byte <= 0x7F;
    }

    static consteval auto char_class_name()
    {
        return "byte";
    }

    static consteval auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        if constexpr (Byte <= 0x7F)
            result.insert(Byte);
        return result;
    }

    static constexpr auto char_class_match_cp([[maybe_unused]] char32_t cp)
    {
        if constexpr (Byte <= 0x7F)
            return std::false_type{};
        else
            return cp == Byte;
    }
};

template <lexy::is_char_class_rule C>
constexpr auto _make_char_class(C c)
{
    return c;
}

template <lexy::_detail::string_literal Str>
consteval bool can_make_char_class() {
    if (Str.size() != 1)
        return false;
    using CharT = lexy::_detail::char_type_t<Str>;
    constexpr auto C = Str.data[0];
    return (C <= 0x7F) || std::is_same_v<CharT, char32_t> || std::is_same_v<CharT, unsigned char>;
}


template <lexy::_detail::string_literal Str>
constexpr auto _make_char_class(_lit<Str>)
{
    constexpr auto C = Str.data[0];
    if constexpr (std::is_same_v<lexy::_detail::char_type_t<Str>, unsigned char>)
        return _cb<C>{};
    else
        return _ccp<static_cast<char32_t>(C)>{};
}

template <lexy::_detail::string_literal Str>
constexpr auto _make_char_class(_lcp<Str>)
{
    return _ccp<Str.data[0]>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename... Cs>
struct _calt : char_class_base<_calt<Cs...>>
{
    static_assert(sizeof...(Cs) > 1);

    static constexpr auto char_class_unicode()
    {
        constexpr auto non_unicode = (!Cs::char_class_unicode() || ...);
        static_assert(!non_unicode
                          // If at least one is non-Unicode, either they all must be non-Unicode or
                          // only match ASCII.
                          || ((!Cs::char_class_unicode()
                               || std::is_same_v<decltype(Cs::char_class_match_cp(0)),
                                                 std::false_type>)&&...),
                      "cannot mix bytes and Unicode char classes");
        return !non_unicode;
    }

    static consteval auto char_class_name()
    {
        return "union";
    }

    static consteval auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        (result.insert(Cs::char_class_ascii()), ...);
        return result;
    }

    static constexpr auto char_class_match_cp(char32_t cp)
    {
        if constexpr ((std::is_same_v<decltype(Cs::char_class_match_cp(cp)), std::false_type>
                       && ...))
            return std::false_type{};
        else
            return (Cs::char_class_match_cp(cp) || ...);
    }
};

template <typename R>
constexpr bool _is_class_rule = false;

template <lexy::is_char_class_rule C>
constexpr bool _is_class_rule<C> = true;

template <lexy::_detail::string_literal Str>
constexpr bool _is_class_rule<_lit<Str>> = can_make_char_class<Str>();

template <lexy::_detail::string_literal Str>
constexpr bool _is_class_rule<_lcp<Str>> =
    (Str.size() == 1) && std::is_same_v<lexy::_detail::char_type_t<Str>, char32_t>;

template <typename R>
concept is_class_rule = _is_class_rule<R>;

template <is_class_rule R1, is_class_rule R2>
constexpr auto operator/(R1 r1, R2 r2)
    -> _calt<decltype(_make_char_class(r1)), decltype(_make_char_class(r2))>
{
    return {};
}

template <typename... Cs, is_class_rule C>
constexpr auto operator/(_calt<Cs...>, C c) -> _calt<Cs..., decltype(_make_char_class(c))>
{
    return {};
}
template <is_class_rule C, typename... Cs>
constexpr auto operator/(C c, _calt<Cs...>) -> _calt<decltype(_make_char_class(c)), Cs...>
{
    return {};
}

template <typename... Cs, typename... Ds>
constexpr auto operator/(_calt<Cs...>, _calt<Ds...>) -> _calt<Cs..., Ds...>
{
    return {};
}
} // namespace lexyd

namespace lexyd
{
template <typename C>
struct _ccomp : char_class_base<_ccomp<C>>
{
    static constexpr auto char_class_unicode()
    {
        return C::char_class_unicode();
    }

    static consteval auto char_class_name()
    {
        return "complement";
    }

    static consteval auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert(0x00, 0x7F);
        result.remove(C::char_class_ascii());
        return result;
    }

    static constexpr auto char_class_match_cp(char32_t cp)
    {
        if (cp <= 0x7F)
            // If we haven't matched an ASCII character so far, this was intentional.
            return false;

        if constexpr (std::is_same_v<decltype(C::char_class_match_cp(cp)), std::false_type>)
            return true;
        else
            return !C::char_class_match_cp(cp);
    }
};

template <is_class_rule C>
constexpr auto operator-(C c) -> _ccomp<decltype(_make_char_class(c))>
{
    return {};
}
template <typename C>
constexpr auto operator-(_ccomp<C>) -> C
{
    return {};
}
} // namespace lexyd

namespace lexyd
{
template <typename Set, typename Minus>
struct _cminus : char_class_base<_cminus<Set, Minus>>
{
    // calt does the correct logic as well, so re-use it.
    static constexpr auto char_class_unicode()
    {
        return _calt<Set, Minus>::char_class_unicode();
    }

    static consteval auto char_class_name()
    {
        return "minus";
    }

    static consteval auto char_class_ascii()
    {
        auto result = Set::char_class_ascii();
        result.remove(Minus::char_class_ascii());
        return result;
    }

    static constexpr auto char_class_match_cp(char32_t cp)
    {
        if constexpr (std::is_same_v<decltype(Set::char_class_match_cp(cp)), std::false_type>)
            return std::false_type{};
        else if constexpr (std::is_same_v<decltype(Minus::char_class_match_cp(cp)),
                                          std::false_type>)
            // We don't match ASCII at this point: they only reach this point if the ASCII table
            // failed.
            return cp > 0x7F && Set::char_class_match_cp(cp);
        else
            // Same as above, no ASCII.
            return cp > 0x7F && Set::char_class_match_cp(cp) && !Minus::char_class_match_cp(cp);
    }
};

template <typename Set, is_class_rule Minus>
constexpr auto operator-(Set, Minus minus)
{
    return _cminus<Set, decltype(_make_char_class(minus))>{};
}

template <typename Set, typename Minus, is_class_rule OtherMinus>
constexpr auto operator-(_cminus<Set, Minus>, OtherMinus other)
{
    return Set{} - _calt<Minus, decltype(_make_char_class(other))>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename... Cs>
struct _cand : char_class_base<_cand<Cs...>>
{
    static_assert(sizeof...(Cs) > 1);

    // calt does the correct logic as well, so re-use it.
    static constexpr auto char_class_unicode()
    {
        return _calt<Cs...>::char_class_unicode();
    }

    static consteval auto char_class_name()
    {
        return "intersection";
    }

    static consteval auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        for (auto c = 0; c <= 0x7F; ++c)
            if ((Cs::char_class_ascii().contains[c] && ...))
                result.insert(c);
        return result;
    }

    static constexpr auto char_class_match_cp(char32_t cp)
    {
        if constexpr ((std::is_same_v<decltype(Cs::char_class_match_cp(cp)), std::false_type>
                       && ...))
            return std::false_type{};
        else
            return (Cs::char_class_match_cp(cp) && ...);
    }
};

template <is_class_rule C1, is_class_rule C2>
constexpr auto operator&(C1 c1, C2 c2)
    -> _cand<decltype(_make_char_class(c1)), decltype(_make_char_class(c2))>
{
    return {};
}

template <typename... Cs, is_class_rule C>
constexpr auto operator&(_cand<Cs...>, C c) -> _cand<Cs..., decltype(_make_char_class(c))>
{
    return {};
}
template <is_class_rule C, typename... Cs>
constexpr auto operator&(C c, _cand<Cs...>) -> _cand<decltype(_make_char_class(c)), Cs...>
{
    return {};
}

template <typename... Cs, typename... Ds>
constexpr auto operator&(_cand<Cs...>, _cand<Ds...>) -> _cand<Cs..., Ds...>
{
    return {};
}
} // namespace lexyd


