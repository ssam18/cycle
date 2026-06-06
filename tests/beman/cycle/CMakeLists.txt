# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

find_package(GTest REQUIRED)

add_executable(beman.cycle.tests.cycle)
target_sources(beman.cycle.tests.cycle PRIVATE cycle.test.cpp)
target_link_libraries(
    beman.cycle.tests.cycle
    PRIVATE beman::cycle GTest::gtest_main
)
if(BEMAN_CYCLE_USE_MODULES)
    set_target_properties(beman.cycle.tests.cycle PROPERTIES CXX_MODULE_STD ON)
endif()

include(GoogleTest)
gtest_discover_tests(beman.cycle.tests.cycle)
