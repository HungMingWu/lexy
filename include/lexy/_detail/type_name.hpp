// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#pragma once

#include <string_view>
#include <lexy/_detail/config.hpp>

namespace lexy::_detail
{

template <typename T>
concept _detect_name_f = requires {
    { T::name() } -> std::convertible_to<std::string_view>;
};

template <typename T>
concept _detect_name_v = requires {
    T::name;
};

constexpr std::string_view try_remove_prefix(std::string_view src, std::string_view prefix)
{
    std::string_view dst(src);
    if (dst.starts_with(prefix)) {
        dst.remove_prefix(prefix.length());
    }
    return dst;
}

template <auto FnPtr, typename Indices = std::make_index_sequence<FnPtr().size()>>
struct _string_view_holder;

template <auto FnPtr, std::size_t... Indices>
struct _string_view_holder<FnPtr, std::index_sequence<Indices...>>
{
    static constexpr auto view = FnPtr();

    static constexpr typename decltype(view)::value_type value[] = {view[Indices]..., {}};
};

template <auto FnPtr>
inline constexpr const auto* make_cstr = _string_view_holder<FnPtr>::value;

template <typename T>
constexpr auto _full_type_name()
{
#if defined(__clang__)
#    define LEXY_HAS_AUTOMATIC_TYPE_NAME 1
#    define LEXY_HAS_CONSTEXPR_AUTOMATIC_TYPE_NAME 1

    constexpr auto prefix = std::string_view("auto lexy::_detail::_full_type_name() [T = ");
    constexpr auto suffix = std::string_view("]");

    auto function = std::string_view(__PRETTY_FUNCTION__);
    function.remove_prefix(prefix.length());
    function.remove_suffix(suffix.length());
    function = try_remove_prefix(function, "(anonymous namespace)::");
    return function;

#elif defined(__GNUC__)
#    define LEXY_HAS_AUTOMATIC_TYPE_NAME 1
#    if __GNUC__ > 8
#        define LEXY_HAS_CONSTEXPR_AUTOMATIC_TYPE_NAME 1
#    else
#        define LEXY_HAS_CONSTEXPR_AUTOMATIC_TYPE_NAME 0
#    endif

    constexpr auto prefix
        = std::string_view("constexpr auto lexy::_detail::_full_type_name() [with T = ");
    constexpr auto suffix = std::string_view("]");

    auto function = std::string_view(__PRETTY_FUNCTION__);
    function.remove_prefix(prefix.length());
    function.remove_suffix(suffix.length());
    function = try_remove_prefix(function, "{anonymous}::");
    return function;

#elif defined(_MSC_VER)
#    define LEXY_HAS_AUTOMATIC_TYPE_NAME 1
#    define LEXY_HAS_CONSTEXPR_AUTOMATIC_TYPE_NAME 1

    constexpr auto prefix = std::string_view("auto __cdecl lexy::_detail::_full_type_name<");
    constexpr auto suffix = std::string_view(">(void)");

    auto function = std::string_view(__FUNCSIG__);
    function.remove_prefix(prefix.length());
    function.remove_suffix(suffix.length());
    function = try_remove_prefix(function, "struct ");
    function = try_remove_prefix(function, "class ");
    function = try_remove_prefix(function, "`anonymous-namespace'::");
    return function;

#else
#    define LEXY_HAS_AUTOMATIC_TYPE_NAME 0
#    define LEXY_HAS_CONSTEXPR_AUTOMATIC_TYPE_NAME 0

    return std::string_view("unknown-type");

#endif
}

template <typename T, int NsCount>
constexpr std::string_view _type_name()
{
    auto name = _full_type_name<T>();
    if (name.find('<') != std::string_view::npos && NsCount != 0)
        return name;

    for (auto namespace_count = NsCount; namespace_count > 0; --namespace_count)
    {
        auto pos = name.find("::");
        if (pos == std::string_view::npos)
            break;
        name.remove_prefix(pos + 2);
    }
    return name;
}

template <typename T, int NsCount = 1>
constexpr const char* type_name()
{
    if constexpr (_detect_name_f<T>)
        return T::name();
    else if constexpr (_detect_name_v<T>)
        return T::name;
    else if constexpr (LEXY_HAS_CONSTEXPR_AUTOMATIC_TYPE_NAME)
        return make_cstr<_type_name<T, NsCount>>;
    else
        return "unknown-type";
}

template <typename T, int NsCount>
inline constexpr const char* _type_id_holder = type_name<T, NsCount>();

// Returns a unique address for each type.
// For implementation reasons, it also doubles as the pointer to the name.
template <typename T, int NsCount = 1>
constexpr const char* const* type_id()
{
    if constexpr (_detect_name_v<T> //
                  && !_detect_name_f<T>)
    {
        // We can use the address of the static constexpr directly.
        return &T::name;
    }
    else
    {
        // We instantiate a variable template with a function unique by type.
        // As the variable is inline, there should be a single address only.
        return &_type_id_holder<T, NsCount>;
    }
}
} // namespace lexy::_detail


