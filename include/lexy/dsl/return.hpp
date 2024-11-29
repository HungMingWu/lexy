// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#pragma once

#include <lexy/action/base.hpp>
#include <lexy/dsl/base.hpp>

namespace lexyd
{
struct _ret : rule_base
{
    // We unconditionally jump to the final parser.
    template <typename NextParser>
    using p = lexy::_detail::final_parser;
};

/// Finishes parsing a production without considering subsequent rules.
constexpr auto return_ = _ret{};
} // namespace lexyd

