# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
include_directories(
    ${PROJECT_SOURCE_DIR}/libs/catch
    include
)

set(SLS_TEST_SOURCES 
    test.cpp
)

add_executable(testclient src/testclient.cpp)
target_link_libraries(testclient slsSupportStatic)
set_target_properties(testclient PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(testserver src/testserver.cpp)
target_link_libraries(testserver slsSupportStatic)
set_target_properties(testserver PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    
add_executable(tests ${SLS_TEST_SOURCES})
target_link_libraries(tests
    PUBLIC 
      slsProjectOptions
      slsSupportStatic
      pthread
      rt
    PRIVATE
      slsProjectWarnings
)  

if (SLS_USE_DETECTOR)
    target_link_libraries(tests PUBLIC
        slsDetectorStatic
    )
endif (SLS_USE_DETECTOR)

if (SLS_USE_RECEIVER)
    target_link_libraries(tests PUBLIC
        slsReceiverStatic
    )
endif (SLS_USE_RECEIVER)
if (SLS_USE_MOENCH)
    target_link_libraries(tests PUBLIC
        tiffio
    )
endif()

if((CMAKE_BUILD_TYPE STREQUAL "Release") AND SLS_LTO_AVAILABLE)
    set_property(TARGET tests PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif()

set_target_properties(tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )

include(CTest)
include(Catch)
catch_discover_tests(tests)

configure_file(scripts/test_simulators.py ${CMAKE_BINARY_DIR}/bin/test_simulators.py COPYONLY)
configure_file(scripts/test_frame_synchronizer.py ${CMAKE_BINARY_DIR}/bin/test_frame_synchronizer.py COPYONLY)
configure_file(scripts/utils_for_test.py ${CMAKE_BINARY_DIR}/bin/utils_for_test.py COPYONLY)
