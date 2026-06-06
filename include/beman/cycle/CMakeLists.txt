# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

if(BEMAN_CYCLE_USE_MODULES)
    target_sources(
        beman.cycle
        PUBLIC
            FILE_SET CXX_MODULES FILES cycle.cppm
            FILE_SET HEADERS
                FILES
                    config.hpp
                    cycle.hpp
                    detail/expo_concept.hpp
                    "${PROJECT_BINARY_DIR}/include/beman/cycle/config_generated.hpp"
    )
else()
    target_sources(
        beman.cycle
        PUBLIC
            FILE_SET HEADERS
                FILES
                    config.hpp
                    cycle.hpp
                    detail/expo_concept.hpp
                    "${PROJECT_BINARY_DIR}/include/beman/cycle/config_generated.hpp"
    )
endif()
