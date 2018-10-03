MESSAGE( STATUS "CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR} )
MESSAGE( STATUS "PROJECT_SOURCE_DIR: " ${PROJECT_SOURCE_DIR} )

# set(SOURCES
#     slsDetectorClient.cpp
# )

include_directories(
    include
    tests
    ${PROJECT_SOURCE_DIR}/catch
)



if(USE_TESTS)
    set(LOCAL_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    set(TEST_SOURCES
        # ${BASE_TEST_DIR}/test-MultiDetectorCaller.cpp
        ${LOCAL_TEST_DIR}/test-container_utils.cpp
        ${LOCAL_TEST_DIR}/test.cpp
        # PARENT_SCOPE
    )
    add_executable(t2 ${TEST_SOURCES})
    set_target_properties(t2 PROPERTIES
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

