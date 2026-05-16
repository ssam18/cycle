# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

find_package(GTest REQUIRED)

add_executable(beman.cycle.tests.cycle)
target_sources(beman.cycle.tests.cycle PRIVATE cycle.test.cpp)
target_link_libraries(
    beman.cycle.tests.cycle
    PRIVATE beman::cycle GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(beman.cycle.tests.cycle)
