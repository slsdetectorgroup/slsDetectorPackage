# include_directories(
#     ${PROJECT_SOURCE_DIR}/catch
# )

# set(SOURCES
#     test.cpp
#     test-GeneralData.cpp
# )

target_sources(tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/test-GeneralData.cpp)

# add_executable(testSlsReceiver ${SOURCES})
# target_link_libraries(testSlsReceiver 
#     slsSupportLib
#     slsDetectorShared
#     slsReceiverShared
#     pthread
#     rt
# )  
# set_target_properties(testSlsReceiver PROPERTIES
#         RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
#     )
# #TODO! Move to automatic test discovery
# add_test(test-testSlsReceiver ${CMAKE_BINARY_DIR}/bin/testSlsReceiver)