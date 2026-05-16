# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

set(ALL_EXAMPLES cycle)
message("Examples to be built: ${ALL_EXAMPLES}")

foreach(example ${ALL_EXAMPLES})
    add_executable(beman.cycle.examples.${example})
    target_sources(beman.cycle.examples.${example} PRIVATE ${example}.cpp)
    target_link_libraries(beman.cycle.examples.${example} PRIVATE beman::cycle)
endforeach()
