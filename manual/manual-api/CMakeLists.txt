
add_executable(slsMultiReceiver
    mainReceiver.cpp
)
set_target_properties(slsMultiReceiver PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
target_link_libraries(slsMultiReceiver
    slsReceiverShared
    pthread
    zmq
    rt
)

install(TARGETS slsMultiReceiver  
        RUNTIME DESTINATION bin)
        