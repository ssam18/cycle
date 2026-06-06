// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/cycle/config.hpp>
#include <gtest/gtest.h>
#include <beman/cycle/cycle.hpp>

#if BEMAN_CYCLE_USE_MODULES()
import std;
#else
    #include <forward_list>
    #include <iterator>
    #include <list>
    #include <ranges>
    #include <type_traits>
    #include <vector>
#endif

namespace cyc = beman::cycle;

// ---------- Empty range ----------

TEST(CycleTest, EmptyVectorIsEmptyView) {
    std::vector<int> v;
    auto             c = v | cyc::views::cycle;
    EXPECT_TRUE(std::ranges::empty(c));
    EXPECT_TRUE(c.begin() == c.end());
}

TEST(CycleTest, EmptyIotaIsEmptyView) {
    auto e = std::views::empty<int>;
    auto c = e | cyc::views::cycle;
    EXPECT_TRUE(std::ranges::empty(c));
}

TEST(CycleTest, NonEmptyIsNotEmpty) {
    std::vector<int> v = {1};
    auto             c = v | cyc::views::cycle;
    EXPECT_FALSE(std::ranges::empty(c));
    EXPECT_FALSE(c.begin() == c.end());
}

// ---------- Forward iteration matches the paper's example ----------

TEST(CycleTest, IotaCycleTake10) {
    auto             c = std::views::iota(0, 3) | cyc::views::cycle | std::views::take(10);
    std::vector<int> got;
    for (int x : c)
        got.push_back(x);
    EXPECT_EQ(got, (std::vector<int>{0, 1, 2, 0, 1, 2, 0, 1, 2, 0}));
}

TEST(CycleTest, NextThenDecrementWrapsToLast) {
    auto cycle = std::views::iota(0, 3) | cyc::views::cycle;
    auto it    = std::ranges::next(cycle.begin(), 4); // points at logical index 4 -> value 1
    EXPECT_EQ(*it, 1);
    --it;
    EXPECT_EQ(*it, 0);
}

// ---------- Random-access subscript wraps around ----------

TEST(CycleTest, SubscriptWrapsForwardAndBackward) {
    std::vector<int> v     = {10, 20, 30};
    auto             cycle = v | cyc::views::cycle;
    for (int i = 0; i < 30; ++i) {
        EXPECT_EQ(cycle[i], v[i % 3]) << "i = " << i;
    }
    // Operator[] forwards to operator+; verify around the cycle boundary explicitly.
    EXPECT_EQ(cycle[42], 10);
    EXPECT_EQ(cycle[43], 20);
    EXPECT_EQ(cycle[44], 30);
    EXPECT_EQ(cycle[49], 20);
}

TEST(CycleTest, PlusEqualNegativeWraps) {
    std::vector<int> v     = {10, 20, 30};
    auto             cycle = v | cyc::views::cycle;
    auto             it    = cycle.begin();
    it += 10; // logical index 10 -> v[1] = 20
    EXPECT_EQ(*it, 20);
    it += -5; // logical index 5 -> v[2] = 30
    EXPECT_EQ(*it, 30);
    it += -7; // logical index -2 -> v[1] = 20 (n_ = -1)
    EXPECT_EQ(*it, 20);
}

TEST(CycleTest, IteratorDifferenceMatchesLogicalDistance) {
    std::vector<int> v     = {10, 20, 30};
    auto             cycle = v | cyc::views::cycle;
    auto             a     = std::ranges::next(cycle.begin(), 0);
    auto             b     = std::ranges::next(cycle.begin(), 7);
    EXPECT_EQ(b - a, 7);
    EXPECT_EQ(a - b, -7);
}

// ---------- end() yields default_sentinel ----------

TEST(CycleTest, EndIsDefaultSentinel) {
    std::vector<int> v = {1, 2};
    auto             c = v | cyc::views::cycle;
    auto             s = c.end();
    static_assert(std::same_as<decltype(s), std::default_sentinel_t>);
    EXPECT_FALSE(c.begin() == s); // non-empty -> begin != end
}

// ---------- Iterator concept and category derive from Base ----------

namespace {

using RAVec     = std::vector<int>;
using BidirList = std::list<int>;
using FwdList   = std::forward_list<int>;

template <class R>
using CycleIterConcept =
    typename decltype(std::declval<cyc::cycle_view<std::views::all_t<R&>>&>().begin())::iterator_concept;

template <class R>
using CycleIterCategory =
    typename decltype(std::declval<cyc::cycle_view<std::views::all_t<R&>>&>().begin())::iterator_category;

} // namespace

TEST(CycleTest, IteratorConceptsByBaseCategory) {
    static_assert(std::same_as<CycleIterConcept<RAVec>, std::random_access_iterator_tag>);
    static_assert(std::same_as<CycleIterConcept<BidirList>, std::bidirectional_iterator_tag>);
    static_assert(std::same_as<CycleIterConcept<FwdList>, std::forward_iterator_tag>);
    SUCCEED();
}

TEST(CycleTest, IteratorCategoriesByBaseCategory) {
    static_assert(std::same_as<CycleIterCategory<RAVec>, std::random_access_iterator_tag>);
    static_assert(std::same_as<CycleIterCategory<BidirList>, std::bidirectional_iterator_tag>);
    static_assert(std::same_as<CycleIterCategory<FwdList>, std::forward_iterator_tag>);
    SUCCEED();
}

// ---------- Iterator equality compares (n_, current_) ----------

TEST(CycleTest, EqualityRequiresSameLap) {
    std::vector<int> v     = {1, 2, 3};
    auto             cycle = v | cyc::views::cycle;
    auto             a     = cycle.begin();                       // n_=0, *=1
    auto             b     = std::ranges::next(cycle.begin(), 3); // n_=1, *=1
    EXPECT_EQ(*a, *b);                                            // same underlying element
    EXPECT_FALSE(a == b);                                         // but different laps -> not equal
    auto c = std::ranges::next(cycle.begin(), 0);
    EXPECT_TRUE(a == c);
}

// ---------- Bidirectional decrement on non-random-access ----------

TEST(CycleTest, BidirectionalListDecrementWraps) {
    std::list<int> l  = {1, 2, 3};
    auto           c  = l | cyc::views::cycle;
    auto           it = c.begin();
    EXPECT_EQ(*it, 1);
    --it; // wrap to last element
    EXPECT_EQ(*it, 3);
    --it;
    EXPECT_EQ(*it, 2);
    --it;
    EXPECT_EQ(*it, 1);
}

// ---------- view_interface forwards correctly (size etc) ----------

TEST(CycleTest, NotBorrowedRange) {
    // The paper explicitly states cycle_view is not a borrowed range.
    static_assert(!std::ranges::borrowed_range<cyc::cycle_view<std::ranges::ref_view<std::vector<int>>>>);
    SUCCEED();
}

TEST(CycleTest, IsForwardRange) {
    std::vector<int> v = {1, 2, 3};
    auto             c = v | cyc::views::cycle;
    static_assert(std::ranges::forward_range<decltype(c)>);
    static_assert(std::ranges::random_access_range<decltype(c)>); // because v is RA + sized
    SUCCEED();
}
