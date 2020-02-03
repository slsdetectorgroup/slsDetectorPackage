target_sources(tests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/test-GeneralData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-FileNames.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test-CircularFifo.cpp
)

target_include_directories(tests PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src>")