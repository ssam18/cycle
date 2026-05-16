// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_CYCLE_CYCLE_HPP
#define BEMAN_CYCLE_CYCLE_HPP

// Implementation of views::cycle as proposed in P3806R0.
// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3806r0.html

#include <compare>
#include <concepts>
#include <iterator>
#include <memory>
#include <ranges>
#include <type_traits>
#include <utility>

namespace beman::cycle {

namespace detail {

template <bool Const, class T>
using maybe_const = std::conditional_t<Const, const T, T>;

template <class V>
concept simple_view = std::ranges::view<V> && std::ranges::range<const V> &&
                      std::same_as<std::ranges::iterator_t<V>, std::ranges::iterator_t<const V>> &&
                      std::same_as<std::ranges::sentinel_t<V>, std::ranges::sentinel_t<const V>>;

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
    using C = typename std::iterator_traits<std::ranges::iterator_t<Base>>::iterator_category;
    if constexpr (std::derived_from<C, std::random_access_iterator_tag> && std::ranges::sized_range<Base>) {
        return std::random_access_iterator_tag{};
    } else if constexpr (std::derived_from<C, std::bidirectional_iterator_tag> && std::ranges::common_range<Base>) {
        return std::bidirectional_iterator_tag{};
    } else {
        return std::forward_iterator_tag{};
    }
}

} // namespace detail

template <std::ranges::view V>
    requires std::ranges::forward_range<V>
class cycle_view : public std::ranges::view_interface<cycle_view<V>> {
  private:
    V base_ = V();

    template <bool Const>
    class iterator;

    template <bool>
    friend class iterator;

  public:
    cycle_view()
        requires std::default_initializable<V>
    = default;

    constexpr explicit cycle_view(V base) : base_(std::move(base)) {}

    constexpr V base() const&
        requires std::copy_constructible<V>
    {
        return base_;
    }
    constexpr V base() && { return std::move(base_); }

    constexpr auto begin()
        requires(!detail::simple_view<V>)
    {
        return iterator<false>(*this, std::ranges::begin(base_));
    }

    constexpr auto begin() const
        requires std::ranges::forward_range<const V>
    {
        return iterator<true>(*this, std::ranges::begin(base_));
    }

    constexpr auto end() const noexcept { return std::default_sentinel; }
};

template <class R>
cycle_view(R&&) -> cycle_view<std::views::all_t<R>>;

template <std::ranges::view V>
    requires std::ranges::forward_range<V>
template <bool Const>
class cycle_view<V>::iterator {
  private:
    using Parent = detail::maybe_const<Const, cycle_view>;
    using Base   = detail::maybe_const<Const, V>;

    std::ranges::iterator_t<Base>         current_ = std::ranges::iterator_t<Base>();
    Parent*                               parent_  = nullptr;
    std::ranges::range_difference_t<Base> n_       = 0;

    friend cycle_view;
    friend class iterator<!Const>;

    constexpr iterator(Parent& parent, std::ranges::iterator_t<Base> current)
        : current_(std::move(current)), parent_(std::addressof(parent)) {}

    // Helpers used by hidden-friend operators. They are member functions of
    // iterator, which is itself a friend of cycle_view, so they can reach
    // cycle_view::base_. Hidden friends defined inside iterator are not
    // automatically friends of cycle_view (MSVC enforces this strictly), so
    // they go through these helpers instead of touching parent_->base_.
    constexpr bool base_is_empty() const { return std::ranges::empty(parent_->base_); }

    constexpr std::ranges::range_difference_t<Base> base_distance() const {
        return std::ranges::distance(parent_->base_);
    }

  public:
    using iterator_concept  = decltype(detail::cycle_iterator_concept_t<Base>());
    using iterator_category = decltype(detail::cycle_iterator_category_t<Base>());
    using value_type        = std::ranges::range_value_t<Base>;
    using difference_type   = std::ranges::range_difference_t<Base>;

    iterator()
        requires std::default_initializable<std::ranges::iterator_t<Base>>
    = default;

    constexpr iterator(iterator<!Const> i)
        requires Const && std::convertible_to<std::ranges::iterator_t<V>, std::ranges::iterator_t<Base>>
        : current_(std::move(i.current_)), parent_(i.parent_), n_(i.n_) {}

    constexpr std::ranges::iterator_t<Base> base() const { return current_; }

    constexpr decltype(auto) operator*() const { return *current_; }

    constexpr std::ranges::iterator_t<Base> operator->() const
        requires detail::has_arrow<std::ranges::iterator_t<Base>>
    {
        return current_;
    }

    constexpr iterator& operator++() {
        if (++current_ == std::ranges::end(parent_->base_)) {
            current_ = std::ranges::begin(parent_->base_);
            ++n_;
        }
        return *this;
    }

    constexpr iterator operator++(int) {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    constexpr iterator& operator--()
        requires detail::bidirectional_common<Base> || detail::sized_random_access_range<Base>
    {
        if (current_ == std::ranges::begin(parent_->base_)) {
            if constexpr (std::ranges::common_range<Base>) {
                current_ = std::ranges::end(parent_->base_);
            } else {
                current_ = std::ranges::begin(parent_->base_) + std::ranges::distance(parent_->base_);
            }
            --n_;
        }
        --current_;
        return *this;
    }

    constexpr iterator operator--(int)
        requires detail::bidirectional_common<Base> || detail::sized_random_access_range<Base>
    {
        auto tmp = *this;
        --*this;
        return tmp;
    }

    constexpr iterator& operator+=(difference_type n)
        requires detail::sized_random_access_range<Base>
    {
        const auto first  = std::ranges::begin(parent_->base_);
        const auto dist   = static_cast<difference_type>(std::ranges::distance(parent_->base_));
        const auto offset = static_cast<difference_type>(current_ - first);
        const auto new_n  = n + offset;
        auto       quot   = new_n / dist;
        auto       rem    = new_n % dist;
        // Adjust toward floor division so the new offset is always in [0, dist).
        if (rem < 0) {
            rem += dist;
            --quot;
        }
        n_ += quot;
        current_ = first + rem;
        return *this;
    }

    constexpr iterator& operator-=(difference_type n)
        requires detail::sized_random_access_range<Base>
    {
        return *this += -n;
    }

    constexpr decltype(auto) operator[](difference_type n) const
        requires detail::sized_random_access_range<Base>
    {
        return *(*this + n);
    }

    friend constexpr bool operator==(const iterator& x, const iterator& y) {
        return x.n_ == y.n_ && x.current_ == y.current_;
    }

    friend constexpr bool operator==(const iterator& x, std::default_sentinel_t) { return x.base_is_empty(); }

    friend constexpr bool operator<(const iterator& x, const iterator& y)
        requires std::ranges::random_access_range<Base>
    {
        if (x.n_ != y.n_) {
            return x.n_ < y.n_;
        }
        return x.current_ < y.current_;
    }

    friend constexpr bool operator>(const iterator& x, const iterator& y)
        requires std::ranges::random_access_range<Base>
    {
        return y < x;
    }

    friend constexpr bool operator<=(const iterator& x, const iterator& y)
        requires std::ranges::random_access_range<Base>
    {
        return !(y < x);
    }

    friend constexpr bool operator>=(const iterator& x, const iterator& y)
        requires std::ranges::random_access_range<Base>
    {
        return !(x < y);
    }

    friend constexpr auto operator<=>(const iterator& x, const iterator& y)
        requires std::ranges::random_access_range<Base> && std::three_way_comparable<std::ranges::iterator_t<Base>>
    {
        using R = std::compare_three_way_result_t<std::ranges::iterator_t<Base>>;
        if (x.n_ != y.n_) {
            return R(x.n_ <=> y.n_);
        }
        return R(x.current_ <=> y.current_);
    }

    friend constexpr iterator operator+(const iterator& i, difference_type n)
        requires detail::sized_random_access_range<Base>
    {
        auto r = i;
        r += n;
        return r;
    }

    friend constexpr iterator operator+(difference_type n, const iterator& i)
        requires detail::sized_random_access_range<Base>
    {
        return i + n;
    }

    friend constexpr iterator operator-(const iterator& i, difference_type n)
        requires detail::sized_random_access_range<Base>
    {
        auto r = i;
        r -= n;
        return r;
    }

    friend constexpr difference_type operator-(const iterator& x, const iterator& y)
        requires std::sized_sentinel_for<std::ranges::iterator_t<Base>, std::ranges::iterator_t<Base>> &&
                 std::ranges::sized_range<Base>
    {
        const auto dist = x.base_distance();
        return (x.n_ - y.n_) * dist + (x.current_ - y.current_);
    }

    friend constexpr std::ranges::range_rvalue_reference_t<Base>
    iter_move(const iterator& i) noexcept(noexcept(std::ranges::iter_move(i.current_))) {
        return std::ranges::iter_move(i.current_);
    }
};

namespace detail {

struct cycle_fn {
    template <std::ranges::viewable_range R>
        requires std::ranges::forward_range<R>
    constexpr auto operator()(R&& r) const {
        return cycle_view<std::views::all_t<R>>(std::views::all(std::forward<R>(r)));
    }

    template <std::ranges::viewable_range R>
        requires std::ranges::forward_range<R>
    friend constexpr auto operator|(R&& r, const cycle_fn& self) {
        return self(std::forward<R>(r));
    }
};

} // namespace detail

namespace views {

inline constexpr detail::cycle_fn cycle{};

} // namespace views

} // namespace beman::cycle

#endif // BEMAN_CYCLE_CYCLE_HPP
