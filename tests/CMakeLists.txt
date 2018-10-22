MESSAGE( STATUS "CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR} )
MESSAGE( STATUS "PROJECT_SOURCE_DIR: " ${PROJECT_SOURCE_DIR} )



include_directories(
    ${PROJECT_SOURCE_DIR}/catch
    ${PROJECT_SOURCE_DIR}/slsSupportLib/include
    ${PROJECT_SOURCE_DIR}/slsDetectorSoftware/multiSlsDetector
)



if(USE_TESTS)
    set(LOCAL_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
    set(TEST_SOURCES
        ${LOCAL_TEST_DIR}/test-container_utils.cpp
        ${LOCAL_TEST_DIR}/test-multiDetector.cpp
        ${LOCAL_TEST_DIR}/test.cpp
        # PARENT_SCOPE
    )
    add_executable(test ${TEST_SOURCES})
    target_link_libraries(test 
        slsDetectorShared
        pthread
    )  
    set_target_properties(test PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        )
endif()



#option(USE_TESTS "Determines whether to build tests." OFF)
# if(USE_TESTS)
#     # Prepare "Catch" library for other executables
#     set(CATCH_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/catch)
#     add_library(Catch INTERFACE)
#     target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

#     # Make test executable
#     add_executable(tests ${BASE_TEST_SOURCES})
#     target_link_libraries(tests Catch)

#     set_target_properties(tests PROPERTIES
#         RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
#     )

#     #enable_testing()
#     #add_test(NAME CommandLineClient COMMAND tests)

# endif()


# install(TARGETS sls_client  DESTINATION bin)

