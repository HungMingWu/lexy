// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include "../../docs/assets/cpp/playground_headers.hpp"

#define LEXY_PLAYGROUND_PRODUCTION production
#include "../../docs/assets/cpp/playground_prefix.cpp" // NOLINT

struct production
{
    static constexpr auto rule = dsl::lit<"hello">;
};

#include "../../docs/assets/cpp/playground_main.cpp" // NOLINT

