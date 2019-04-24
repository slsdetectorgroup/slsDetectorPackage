target_sources(tests PRIVATE 
                ${CMAKE_CURRENT_SOURCE_DIR}/test-ClientInterface.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/test-CmdLineParser.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/test-container_utils.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/test-network_utils.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/test-string_utils.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/test-Timer.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/test-sls_detector_defs.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/test-Sockets.cpp
                )