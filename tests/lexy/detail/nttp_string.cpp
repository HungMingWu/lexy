// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <string_view>
#include <lexy/_detail/nttp_string.hpp>

#include <doctest/doctest.h>

TEST_CASE("nttp_string")
{
    using string = lexy::_detail::type_string<"abc">;
    CHECK(std::is_same_v<typename string::char_type, char>);
    CHECK(string::c_str() == std::string_view("abc"));
    CHECK(string::c_str<wchar_t>() == std::wstring_view(L"abc"));

    using wstring = lexy::_detail::type_string<L"abc">;
    CHECK(std::is_same_v<typename wstring::char_type, wchar_t>);
    CHECK(wstring::c_str() == std::wstring_view(L"abc"));
    CHECK(wstring::c_str<char>() == std::string_view("abc"));
}

