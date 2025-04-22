# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
target_sources(tests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/test-GeneralData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-CircularFifo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-ArrangeDataBasedOnBitList.cpp
)

target_include_directories(tests PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src>")

message(STATUS "Resolved path: ${CMAKE_CURRENT_SOURCE_DIR}/../src")