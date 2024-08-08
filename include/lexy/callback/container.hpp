// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_CALLBACK_CONTAINER_HPP_INCLUDED
#define LEXY_CALLBACK_CONTAINER_HPP_INCLUDED

#include <lexy/callback/base.hpp>

namespace lexy
{
struct nullopt;

template <typename Container>
concept _has_reserve = requires {
    std::declval<Container&>().reserve(std::size_t());
};

template <typename Container>
concept _has_append = requires {
    std::declval<Container&>().append(std::declval<Container&&>());
};

} // namespace lexy

//=== as_list ===//
namespace lexy
{
template <typename Container>
struct _list_sink
{
    Container _result;

    using return_type = Container;

    template <typename C = Container, typename U>
    constexpr auto operator()(U&& obj) -> decltype(std::declval<C&>().push_back(std::forward<U>(obj)))
    {
        return _result.push_back(std::forward<U>(obj));
    }

    template <typename C = Container, typename... Args>
    constexpr auto operator()(Args&&... args)
        -> decltype(std::declval<C&>().emplace_back(std::forward<Args>(args)...))
    {
        return _result.emplace_back(std::forward<Args>(args)...);
    }

    constexpr Container&& finish() &&
    {
        return std::move(_result);
    }
};

template <typename Container, typename AllocFn>
struct _list_alloc
{
    AllocFn _alloc;

    using return_type = Container;

    template <typename State>
    struct _with_state
    {
        State&         _state;
        const AllocFn& _alloc;

        constexpr Container operator()(Container&& container) const
        {
            return std::move(container);
        }
        constexpr Container operator()(nullopt&&) const
        {
            return Container(std::invoke(_alloc, _state));
        }

        template <typename... Args>
        constexpr auto operator()(Args&&... args) const
            -> LEXY_DECAY_DECLTYPE((std::declval<Container&>().push_back(std::forward<Args>(args)), ...),
                                   std::declval<Container>())
        {
            Container result(std::invoke(_alloc, _state));
            if constexpr (_has_reserve<Container>)
                result.reserve(sizeof...(args));
            (result.emplace_back(std::forward<Args>(args)), ...);
            return result;
        }
    };

    template <typename State>
    constexpr auto operator[](State& state) const
    {
        return _with_state<State>{state, _alloc};
    }

    template <typename State>
    constexpr auto sink(State& state) const
    {
        return _list_sink<Container>{Container(std::invoke(_alloc, state))};
    }
};

template <typename Container>
struct _list
{
    using return_type = Container;

    constexpr Container operator()(Container&& container) const
    {
        return std::move(container);
    }
    constexpr Container operator()(nullopt&&) const
    {
        return Container();
    }

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const
        -> LEXY_DECAY_DECLTYPE((std::declval<Container&>().push_back(std::forward<Args>(args)), ...),
                               std::declval<Container>())
    {
        Container result;
        if constexpr (_has_reserve<Container>)
            result.reserve(sizeof...(args));
        (result.emplace_back(std::forward<Args>(args)), ...);
        return result;
    }
    template <typename C = Container, typename... Args>
    constexpr auto operator()(const typename C::allocator_type& allocator, Args&&... args) const
        -> decltype((std::declval<C&>().push_back(std::forward<Args>(args)), ...), C(allocator))
    {
        Container result(allocator);
        if constexpr (_has_reserve<Container>)
            result.reserve(sizeof...(args));
        (result.emplace_back(std::forward<Args>(args)), ...);
        return result;
    }

    constexpr auto sink() const
    {
        return _list_sink<Container>{Container()};
    }
    template <typename C = Container>
    constexpr auto sink(const typename C::allocator_type& allocator) const
    {
        return _list_sink<Container>{Container(allocator)};
    }

    template <typename AllocFn>
    constexpr auto allocator(AllocFn alloc_fn) const
    {
        return _list_alloc<Container, AllocFn>{alloc_fn};
    }
    constexpr auto allocator() const
    {
        return allocator([](const auto& alloc) { return alloc; });
    }
};

/// A callback with sink that creates a list of things (e.g. a `std::vector`, `std::list`, etc.).
/// It repeatedly calls `push_back()` and `emplace_back()`.
template <typename Container>
constexpr auto as_list = _list<Container>{};
} // namespace lexy

//=== as_collection ===//
namespace lexy
{
template <typename Container>
struct _collection_sink
{
    Container _result;

    using return_type = Container;

    template <typename C = Container, typename U>
    constexpr auto operator()(U&& obj) -> decltype(std::declval<C&>().insert(std::forward<U>(obj)))
    {
        return _result.insert(std::forward<U>(obj));
    }

    template <typename C = Container, typename... Args>
    constexpr auto operator()(Args&&... args)
        -> decltype(std::declval<C&>().emplace(std::forward<Args>(args)...))
    {
        return _result.emplace(std::forward<Args>(args)...);
    }

    constexpr Container&& finish() &&
    {
        return std::move(_result);
    }
};

template <typename Container, typename AllocFn>
struct _collection_alloc
{
    AllocFn _alloc;

    using return_type = Container;

    template <typename State>
    struct _with_state
    {
        State&         _state;
        const AllocFn& _alloc;

        constexpr Container operator()(Container&& container) const
        {
            return std::move(container);
        }
        constexpr Container operator()(nullopt&&) const
        {
            return Container(std::invoke(_alloc, _state));
        }

        template <typename... Args>
        constexpr auto operator()(Args&&... args) const
            -> LEXY_DECAY_DECLTYPE((std::declval<Container&>().insert(std::forward<Args>(args)), ...),
                                   std::declval<Container>())
        {
            Container result(std::invoke(_alloc, _state));
            if constexpr (_has_reserve<Container>)
                result.reserve(sizeof...(args));
            (result.emplace(std::forward<Args>(args)), ...);
            return result;
        }
    };

    template <typename State>
    constexpr auto operator[](State& state) const
    {
        return _with_state<State>{state, _alloc};
    }

    template <typename State>
    constexpr auto sink(State& state) const
    {
        return _collection_sink<Container>{Container(std::invoke(_alloc, state))};
    }
};

template <typename Container>
struct _collection
{
    using return_type = Container;

    constexpr Container operator()(Container&& container) const
    {
        return std::move(container);
    }
    constexpr Container operator()(nullopt&&) const
    {
        return Container();
    }

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const
        -> LEXY_DECAY_DECLTYPE((std::declval<Container&>().insert(std::forward<Args>(args)), ...),
                               std::declval<Container>())
    {
        Container result;
        if constexpr (_has_reserve<Container>)
            result.reserve(sizeof...(args));
        (result.emplace(std::forward<Args>(args)), ...);
        return result;
    }

    template <typename C = Container, typename... Args>
    constexpr auto operator()(const typename C::allocator_type& allocator, Args&&... args) const
        -> decltype((std::declval<C&>().insert(std::forward<Args>(args)), ...), C(allocator))
    {
        Container result(allocator);
        if constexpr (_has_reserve<Container>)
            result.reserve(sizeof...(args));
        (result.emplace(std::forward<Args>(args)), ...);
        return result;
    }

    constexpr auto sink() const
    {
        return _collection_sink<Container>{Container()};
    }
    template <typename C = Container>
    constexpr auto sink(const typename C::allocator_type& allocator) const
    {
        return _collection_sink<Container>{Container(allocator)};
    }

    template <typename AllocFn>
    constexpr auto allocator(AllocFn alloc_fn) const
    {
        return _collection_alloc<Container, AllocFn>{alloc_fn};
    }
    constexpr auto allocator() const
    {
        return allocator([](const auto& alloc) { return alloc; });
    }
};

/// A callback with sink that creates an unordered collection of things (e.g. a `std::set`,
/// `std::unordered_map`, etc.). It repeatedly calls `insert()` and `emplace()`.
template <typename T>
constexpr auto as_collection = _collection<T>{};
} // namespace lexy

//=== concat ===//
namespace lexy
{
template <typename Container>
struct _concat
{
    using return_type = Container;

    constexpr Container operator()(nullopt&&) const
    {
        return Container();
    }

    template <typename... Tail>
    constexpr Container _call(Container&& head, Tail&&... tail) const
    {
        if constexpr (sizeof...(Tail) == 0)
            return std::move(head);
        else
        {
            if constexpr (_has_reserve<Container>)
            {
                auto total_size = (head.size() + ... + tail.size());
                head.reserve(total_size);
            }

            auto append = [&head](Container&& container) {
                if constexpr (_has_append<Container>)
                {
                    head.append(std::move(container));
                }
                else
                {
                    for (auto& elem : container)
                        head.push_back(std::move(elem));
                }
            };
            (append(std::move(tail)), ...);

            return std::move(head);
        }
    }

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const -> decltype(_call(Container(std::forward<Args>(args))...))
    {
        return _call(Container(std::forward<Args>(args))...);
    }

    struct _sink
    {
        Container _result;

        using return_type = Container;

        constexpr void operator()(Container&& container)
        {
            if (_result.empty())
            {
                // We assign until we have items.
                // That way we get the existing allocator.
                _result = std::move(container);
            }
            else if constexpr (_has_append<Container>)
            {
                _result.append(std::move(container));
            }
            else
            {
                if constexpr (_has_reserve<Container>)
                {
                    auto capacity   = _result.capacity();
                    auto total_size = _result.size() + container.size();
                    if (total_size > capacity)
                    {
                        // If we need more space we reserve at least twice as much.
                        auto exp_capacity = 2 * capacity;
                        if (total_size > exp_capacity)
                            _result.reserve(total_size);
                        else
                            _result.reserve(exp_capacity);
                    }
                }

                for (auto& elem : container)
                    _result.push_back(std::move(elem));
            }
        }

        constexpr Container&& finish() &&
        {
            return std::move(_result);
        }
    };

    constexpr auto sink() const
    {
        return _sink{};
    }
};

template <typename Container>
constexpr auto concat = _concat<Container>{};
} // namespace lexy

//=== collect ===//
namespace lexy
{
template <typename Container, typename Callback>
class _collect_sink
{
public:
    constexpr explicit _collect_sink(Callback callback) : _callback(std::move(callback)) {}
    template <typename C = Container>
    constexpr explicit _collect_sink(Callback callback, const typename C::allocator_type& allocator)
    : _result(allocator), _callback(std::move(callback))
    {}

    using return_type = Container;

    template <typename... Args>
    constexpr auto operator()(Args&&... args)
        -> decltype(void(std::declval<Callback>()(std::forward<Args>(args)...)))
    {
        _result.push_back(_callback(std::forward<Args>(args)...));
    }

    constexpr auto finish() &&
    {
        return std::move(_result);
    }

private:
    Container                  _result;
    LEXY_EMPTY_MEMBER Callback _callback;
};
template <typename Callback>
class _collect_sink<void, Callback>
{
public:
    constexpr explicit _collect_sink(Callback callback) : _count(0), _callback(std::move(callback))
    {}

    using return_type = std::size_t;

    template <typename... Args>
    constexpr auto operator()(Args&&... args)
        -> decltype(void(std::declval<Callback>()(std::forward<Args>(args)...)))
    {
        _callback(std::forward<Args>(args)...);
        ++_count;
    }

    constexpr auto finish() &&
    {
        return _count;
    }

private:
    std::size_t                _count;
    LEXY_EMPTY_MEMBER Callback _callback;
};

template <typename Container, typename Callback>
class _collect
{
public:
    constexpr explicit _collect(Callback callback) : _callback(std::move(callback)) {}

    constexpr auto sink() const
    {
        return _collect_sink<Container, Callback>(_callback);
    }
    template <typename C = Container>
    constexpr auto sink(const typename C::allocator_type& allocator) const
    {
        return _collect_sink<Container, Callback>(_callback, allocator);
    }

private:
    LEXY_EMPTY_MEMBER Callback _callback;
};

/// Returns a sink that invokes the void-returning callback multiple times, resulting in the number
/// of times it was invoked.
template <typename Callback>
constexpr auto collect(Callback&& callback)
{
    using callback_t = std::decay_t<Callback>;
    static_assert(std::is_void_v<typename callback_t::return_type>,
                  "need to specify a container to collect into for non-void callbacks");
    return _collect<void, callback_t>(std::forward<Callback>(callback));
}

/// Returns a sink that invokes the callback multiple times, storing each result in the container.
template <typename Container, typename Callback>
constexpr auto collect(Callback&& callback)
{
    using callback_t = std::decay_t<Callback>;
    static_assert(!std::is_void_v<typename callback_t::return_type>,
                  "cannot collect a void callback into a container");
    return _collect<Container, callback_t>(std::forward<Callback>(callback));
}
} // namespace lexy

#endif // LEXY_CALLBACK_CONTAINER_HPP_INCLUDED

