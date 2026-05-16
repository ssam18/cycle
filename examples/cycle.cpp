// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Mirrors the example program in P3806R0 (https://godbolt.org/z/3xo1vGs3f),
// adapted to standard C++23 iostream output instead of std::print so that it
// compiles on toolchains where <print> is not yet available.

#include <beman/cycle/cycle.hpp>

#include <iostream>
#include <ranges>

namespace cyc = beman::cycle;

int main() {
    auto ints     = std::views::iota(0, 3);
    auto cycle    = ints | cyc::views::cycle;
    auto cycle_10 = cycle | std::views::take(10);

    std::cout << "[";
    bool first = true;
    for (int v : cycle_10) {
        std::cout << (first ? "" : ", ") << v;
        first = false;
    }
    std::cout << "]\n"; // prints [0, 1, 2, 0, 1, 2, 0, 1, 2, 0]

    auto it = std::ranges::next(cycle.begin(), 4);
    --it;
    std::cout << *it << '\n'; // prints 0

    for (int i = 42; i < 50; ++i) {
        std::cout << cycle[i] << ' '; // prints 0 1 2 0 1 2 0 1
    }
    std::cout << '\n';
}
