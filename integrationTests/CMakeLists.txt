# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
# MESSAGE( STATUS "CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR} )
# MESSAGE( STATUS "PROJECT_SOURCE_DIR: " ${PROJECT_SOURCE_DIR} )



# include_directories(
#     ${PROJECT_SOURCE_DIR}/catch
# )

# target_sources(tests PRIVATE 
#     ${CMAKE_CURRENT_SOURCE_DIR}/test-integrationMulti.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/test-integrationDectector.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/test-eigerIntegration.cpp
# )

# if(SLS_USE_TESTS)
#     set(TEST_SOURCES
#         src/test-slsDetector.cpp
#         src/test.cpp
#     )
#     add_executable(detector_test ${TEST_SOURCES})
    
#     target_link_libraries(detector_test 
#         slsDetectorShared
#         slsProjectOptions
#         slsProjectWarnings
#         slsSupportLib
#         pthread
#         rt
#     )  
#     set_target_properties(detector_test PROPERTIES
#             RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
#     )



# endif()

