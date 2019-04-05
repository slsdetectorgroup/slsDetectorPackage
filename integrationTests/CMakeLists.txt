MESSAGE( STATUS "CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR} )
MESSAGE( STATUS "PROJECT_SOURCE_DIR: " ${PROJECT_SOURCE_DIR} )



include_directories(
    ${PROJECT_SOURCE_DIR}/catch
)

if(SLS_USE_TESTS)
    set(TEST_SOURCES
        src/test-slsDetector.cpp
        src/test.cpp
    )
    add_executable(detector_test ${TEST_SOURCES})
    
    target_link_libraries(detector_test 
        slsDetectorShared
        slsSupportLib
        pthread
        rt
    )  
    set_target_properties(detector_test PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    add_executable(a src/a.cpp)

    target_link_libraries(a 
        slsDetectorShared
        slsSupportLib
        pthread
        rt
    )  
    set_target_properties(a PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )

endif()

