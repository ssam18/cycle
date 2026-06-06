// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_CYCLE_DETAIL_EXPO_CONCEPT_HPP
#define BEMAN_CYCLE_DETAIL_EXPO_CONCEPT_HPP

#include <beman/cycle/config.hpp>

#if BEMAN_CYCLE_USE_MODULES() && !defined(BEMAN_CYCLE_INCLUDED_FROM_INTERFACE_UNIT)

import beman.cycle;

#else

    // Exposition-only concepts and helpers used to implement views::cycle.
    // See P3806R0: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3806r0.html

    #if !BEMAN_CYCLE_USE_MODULES()
        #include <concepts>
        #include <iterator>
        #include <ranges>
        #include <type_traits>
    #endif

namespace beman::cycle::detail {

template <bool Const, class T>
using maybe_const = std::conditional_t<Const, const T, T>;

template <class V>
concept simple_view = std::ranges::view<V> && std::ranges::range<const V> &&
                      std::same_as<std::ranges::iterator_t<V>, std::ranges::iterator_t<const V> > &&
                      std::same_as<std::ranges::sentinel_t<V>, std::ranges::sentinel_t<const V> >;

template <class T>
concept has_arrow = std::input_iterator<T> && (std::is_pointer_v<T> || requires(T t) { t.operator->(); });

template <class R>
concept bidirectional_common = std::ranges::bidirectional_range<R> && std::ranges::common_range<R>;

template <class R>
concept sized_random_access_range = std::ranges::random_access_range<R> && std::ranges::sized_range<R>;

template <class Base>
consteval auto cycle_iterator_concept_t() {
    if constexpr (sized_random_access_range<Base>) {
        return std::random_access_iterator_tag{};
    } else if constexpr (bidirectional_common<Base>) {
        return std::bidirectional_iterator_tag{};
    } else {
        return std::forward_iterator_tag{};
    }
}

template <class Base>
consteval auto cycle_iterator_category_t() {
    using C = typename std::iterator_traits<std::ranges::iterator_t<Base> >::iterator_category;
    if constexpr (std::derived_from<C, std::random_access_iterator_tag> && std::ranges::sized_range<Base>) {
        return std::random_access_iterator_tag{};
    } else if constexpr (std::derived_from<C, std::bidirectional_iterator_tag> && std::ranges::common_range<Base>) {
        return std::bidirectional_iterator_tag{};
    } else {
        return std::forward_iterator_tag{};
    }
}

} // namespace beman::cycle::detail

#endif // BEMAN_CYCLE_USE_MODULES() &&
       // !defined(BEMAN_CYCLE_INCLUDED_FROM_INTERFACE_UNIT)

#endif // BEMAN_CYCLE_DETAIL_EXPO_CONCEPT_HPP
