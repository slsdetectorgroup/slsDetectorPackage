include_directories(
    ${PROJECT_SOURCE_DIR}/libs/catch
    include
)

set(SLS_TEST_SOURCES 
    test.cpp
)

add_executable(testclient src/testclient.cpp)
target_link_libraries(testclient slsSupportShared)
set_target_properties(testclient PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(testserver src/testserver.cpp)
target_link_libraries(testserver slsSupportShared)
set_target_properties(testserver PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    
add_executable(tests ${SLS_TEST_SOURCES})
target_link_libraries(tests 
    slsProjectOptions
    slsSupportShared
    pthread
    rt
    PRIVATE
      slsProjectWarnings
)  

if (SLS_USE_TEXTCLIENT)
    target_link_libraries(tests
        slsDetectorShared
    )
endif (SLS_USE_TEXTCLIENT)

if (SLS_USE_RECEIVER)
    target_link_libraries(tests
        slsReceiverShared
    )
endif (SLS_USE_RECEIVER)


set_target_properties(tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )

include(CTest)
include(Catch)
catch_discover_tests(tests)
