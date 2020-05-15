target_sources(tests PRIVATE 
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
                )