// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

#if defined(LEXY_USER_CONFIG_HEADER)
#    include LEXY_USER_CONFIG_HEADER
#elif defined(__has_include)
#    if __has_include(<lexy_user_config.hpp>)
#        include <lexy_user_config.hpp>
#    elif __has_include("lexy_user_config.hpp")
#        include "lexy_user_config.hpp"
#    endif
#endif

#ifndef LEXY_HAS_UNICODE_DATABASE
#    define LEXY_HAS_UNICODE_DATABASE 0
#endif

#ifndef LEXY_EXPERIMENTAL
#    define LEXY_EXPERIMENTAL 0
#endif

namespace lexy::_detail
{
template <typename T>
std::add_rvalue_reference_t<T> declval();

template <typename T, typename U>
constexpr bool is_decayed_same = std::is_same_v<std::decay_t<T>, std::decay_t<U>>;

template <typename T, typename Fallback>
using type_or = std::conditional_t<std::is_void_v<T>, Fallback, T>;
} // namespace lexy::_detail


