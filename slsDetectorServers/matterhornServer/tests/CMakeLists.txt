
target_sources(tests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/test-HelperFunctions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/utils/HelperFunctions.cpp
)

target_include_directories(tests 
    PUBLIC 
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src/utils>")
