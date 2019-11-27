target_sources(tests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/test-SharedMemory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-slsDetector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-CmdProxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-CmdProxy-rx.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-CmdProxy-eiger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-CmdProxy-jungfrau.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-Result.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-CmdParser.cpp
)

target_include_directories(tests PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src>")