

include_directories(
    ${PROJECT_SOURCE_DIR}/catch
)

set(SOURCES
    test.cpp
    test-SharedMemory.cpp
)

add_executable(testSlsDetector ${SOURCES})
target_link_libraries(testSlsDetector 
    slsSupportLib
    slsDetectorShared
    pthread
    rt
)  
set_target_properties(testSlsDetector PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
#TODO! Move to automatic test discovery
add_test(test-testSlsDetector ${CMAKE_BINARY_DIR}/bin/testSlsDetector)