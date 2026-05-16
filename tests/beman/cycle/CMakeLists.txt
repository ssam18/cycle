# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

find_package(GTest REQUIRED)

add_executable(beman.cycle.tests.todo)
target_sources(beman.cycle.tests.todo PRIVATE todo.test.cpp)
target_link_libraries(
    beman.cycle.tests.todo
    PRIVATE beman::cycle GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(beman.cycle.tests.todo)
