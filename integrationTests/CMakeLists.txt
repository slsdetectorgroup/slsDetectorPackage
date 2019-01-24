MESSAGE( STATUS "CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR} )
MESSAGE( STATUS "PROJECT_SOURCE_DIR: " ${PROJECT_SOURCE_DIR} )



include_directories(
    ${PROJECT_SOURCE_DIR}/catch
)


if(USE_TESTS)

    set(TEST_SOURCES
        src/test-slsDetector.cpp
        # src/test.cpp
    )
    add_executable(a ${TEST_SOURCES})
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

