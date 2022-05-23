# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
target_sources(tests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/test-SharedMemory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-slsDetector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-CmdProxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-CmdProxy-rx.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-CmdProxy-pattern.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-CmdProxy-eiger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-CmdProxy-jungfrau.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-CmdProxy-mythen3.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-CmdProxy-gotthard2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-CmdProxy-gotthard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-CmdProxy-chiptestboard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-CmdProxy-moench.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-CmdProxy-global.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-Result.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-CmdParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-Module.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-Pattern.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-CtbConfig.cpp
)

target_include_directories(tests 
    PUBLIC 
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src>"
    PRIVATE
    ${SLS_INTERNAL_RAPIDJSON_DIR}
)