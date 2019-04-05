include_directories(
    ${PROJECT_SOURCE_DIR}/catch
)

set(SLS_TEST_SOURCES 
    test.cpp
)

add_executable(tests ${SLS_TEST_SOURCES})
target_link_libraries(tests 
    slsProjectOptions
    slsProjectWarnings
    slsSupportLib
    slsDetectorShared
    slsReceiverShared
    pthread
    rt
)  
set_target_properties(tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )

# #TODO! Move to automatic test discovery
# add_test(test ${CMAKE_BINARY_DIR}/bin/testSlsReceiver)