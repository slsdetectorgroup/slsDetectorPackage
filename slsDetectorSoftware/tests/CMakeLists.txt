target_sources(tests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/test-SharedMemory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-slsDetector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-multiSlsDetector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-multiSlsDetectorClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-Result.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-CmdLineParser.cpp
)

target_include_directories(tests PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src>")