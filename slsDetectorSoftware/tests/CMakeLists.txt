# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package


target_sources(tests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/test-SharedMemory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-slsDetector.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/Caller/test-Caller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Caller/test-Caller-rx.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Caller/test-Caller-pattern.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Caller/test-Caller-eiger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Caller/test-Caller-jungfrau.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Caller/test-Caller-mythen3.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Caller/test-Caller-gotthard2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Caller/test-Caller-gotthard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Caller/test-Caller-chiptestboard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Caller/test-Caller-xilinx-chiptestboard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Caller/test-Caller-moench.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Caller/test-Caller-global.cpp

    
    ${CMAKE_CURRENT_SOURCE_DIR}/test-Result.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-CmdParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-Module.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-Pattern.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-CtbConfig.cpp
    


)

target_include_directories(tests 
    PUBLIC 
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include/sls>"
    PRIVATE
    ${SLS_INTERNAL_RAPIDJSON_DIR}
)