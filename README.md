# beman.cycle: Range adaptor that endlessly repeats a forward range (views::cycle, P3806R0).

<!--
SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
-->

<!-- markdownlint-disable-next-line line-length -->
![Library Status](https://raw.githubusercontent.com/bemanproject/beman/refs/heads/main/images/badges/beman_badge-beman_library_under_development.svg) ![Continuous Integration Tests](https://github.com/bemanproject/cycle/actions/workflows/ci_tests.yml/badge.svg) ![Lint Check (pre-commit)](https://github.com/bemanproject/cycle/actions/workflows/pre-commit-check.yml/badge.svg) [![Coverage](https://coveralls.io/repos/github/bemanproject/cycle/badge.svg?branch=main)](https://coveralls.io/github/bemanproject/cycle?branch=main) ![Standard Target](https://github.com/bemanproject/beman/blob/main/images/badges/cpp29.svg)

`beman.cycle` is a minimal C++ library conforming to [The Beman Standard](https://github.com/bemanproject/beman/blob/main/docs/beman_standard.md).

**Implements**: `std::ranges::views::cycle` proposed in [views::cycle (P3806R0)](https://wg21.link/P3806R0).

**Status**: [Under development and not yet ready for production use.](https://github.com/bemanproject/beman/blob/main/docs/beman_library_maturity_model.md#under-development-and-not-yet-ready-for-production-use)

## License

`beman.cycle` is licensed under the Apache License v2.0 with LLVM Exceptions.

## Usage

`beman::cycle::views::cycle` is a range adaptor that takes any non-empty
forward range and yields an infinite view that endlessly repeats the source
range's elements. Applied to an empty range, it yields an empty view.

```c++
#include <beman/cycle/cycle.hpp>

#include <ranges>
#include <vector>

int main() {
    std::vector v = {1, 2, 3};
    for (int x : v | beman::cycle::views::cycle | std::views::take(7)) {
        // 1 2 3 1 2 3 1
    }
}
```

Iterator capabilities track the underlying range:

| Base range                                    | `cycle_view` iterator        |
|-----------------------------------------------|------------------------------|
| `forward_range`                               | forward                      |
| `bidirectional_range` and `common_range`      | bidirectional                |
| `random_access_range` and `sized_range`       | random access (with `[i]`)   |

Notes consistent with P3806R0:

* `views::cycle(empty_range)` is well-formed and yields an empty view.
* `end()` returns `std::default_sentinel`; on a non-empty base the view is infinite.
* `cycle_view` is **not** a borrowed range.
* `iter_swap` is intentionally not provided (two iterators can alias the same base position).

Full runnable examples can be found in [`examples/`](examples/).

## Dependencies

### Build Environment

This project requires at least the following to build:

* A C++ compiler that conforms to the C++23 standard or greater
* CMake 3.30 or later
* (Test Only) GoogleTest

You can disable building tests by setting CMake option `BEMAN_CYCLE_BUILD_TESTS` to
`OFF` when configuring the project.

### Supported Platforms

| Compiler   | Version | C++ Standards | Standard Library  |
|------------|---------|---------------|-------------------|
| GCC        | 15-13   | C++26-C++23   | libstdc++         |
| GCC        | 12      | C++23         | libstdc++         |
| Clang      | 22-19   | C++26-C++23   | libstdc++, libc++ |
| Clang      | 18-17   | C++26-C++23   | libc++            |
| AppleClang | latest  | C++26-C++23   | libc++            |
| MSVC       | latest  | C++23         | MSVC STL          |

## Development

See the [Contributing Guidelines](CONTRIBUTING.md).

## Integrate beman.cycle into your project

### Build

You can build cycle using a CMake workflow preset:

```bash
cmake --workflow --preset gcc-release
```

To list available workflow presets, you can invoke:

```bash
cmake --list-presets=workflow
```

For details on building beman.cycle without using a CMake preset, refer to the
[Contributing Guidelines](CONTRIBUTING.md).

### Installation

To install beman.cycle globally after building with the `gcc-release` preset, you can
run:

```bash
sudo cmake --install build/gcc-release
```

Alternatively, to install to a prefix, for example `/opt/beman`, you can run:

```bash
sudo cmake --install build/gcc-release --prefix /opt/beman
```

This will generate the following directory structure:

```txt
/opt/beman
├── include
│   └── beman
│       └── cycle
│           ├── cycle.hpp
│           └── ...
└── lib
    └── cmake
        └── beman.cycle
            ├── beman.cycle-config-version.cmake
            ├── beman.cycle-config.cmake
            └── beman.cycle-targets.cmake
```

### CMake Configuration

If you installed beman.cycle to a prefix, you can specify that prefix to your CMake
project using `CMAKE_PREFIX_PATH`; for example, `-DCMAKE_PREFIX_PATH=/opt/beman`.

You need to bring in the `beman.cycle` package to define the `beman::cycle` CMake
target:

```cmake
find_package(beman.cycle REQUIRED)
```

You will then need to add `beman::cycle` to the link libraries of any libraries or
executables that include `beman.cycle` headers.

```cmake
target_link_libraries(yourlib PUBLIC beman::cycle)
```

### Using beman.cycle

To use `beman.cycle` in your C++ project,
include an appropriate `beman.cycle` header from your source code.

```c++
#include <beman/cycle/cycle.hpp>
```

> [!NOTE]
>
> `beman.cycle` headers are to be included with the `beman/cycle/` prefix.
> Altering include search paths to spell the include target another way (e.g.
> `#include <cycle.hpp>`) is unsupported.
