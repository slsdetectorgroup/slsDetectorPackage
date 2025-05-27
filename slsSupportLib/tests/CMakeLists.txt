# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
target_sources(tests PRIVATE 
                ${CMAKE_CURRENT_SOURCE_DIR}/test-bit_utils.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/test-file_utils.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/test-container_utils.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/test-network_utils.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/test-string_utils.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/test-Timer.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/test-sls_detector_defs.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/test-Sockets.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/test-StaticVector.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/test-ToString.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/test-TypeTraits.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/test-UdpRxSocket.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/test-logger.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/test-Version.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/test-ZmqSocket.cpp
                )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test-file_utils-channels.txt ${CMAKE_BINARY_DIR}/bin/test-file_utils-channels.txt COPYONLY)               