

include_directories(
    ${PROJECT_SOURCE_DIR}/catch
    ../include
)

set(SOURCES
    test.cpp
    test-ClientInterface.cpp
    test-CmdLineParser.cpp
    test-container_utils.cpp
    test-network_utils.cpp
    test-string_utils.cpp
    test-Timer.cpp
)

add_executable(testSlsSupportLib ${SOURCES})
target_link_libraries(testSlsSupportLib 
    slsSupportLib
    pthread
    rt
)  
set_target_properties(testSlsSupportLib PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
#TODO! Move to automatic test discovery
add_test(test-slsSupportLib ${CMAKE_BINARY_DIR}/bin/testSlsSupportLib)