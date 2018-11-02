set(SOURCES
    mainReceiver.cpp
)

include_directories(
    ../../slsSupportLib/include
    ../../slsReceiverSoftware/include
    ../../build/bin 
    ../../slsdetectorSoftware/slsDetector 
)

add_executable(slsMultiReceiver
    ${SOURCES}
)

target_link_libraries(slsMultiReceiver
    slsReceiverShared
    pthread
    zmq
    rt
)

if (HDF5_FOUND)
    target_link_libraries(slsMultiReceiver ${HDF5_LIBRARIES})
endif ()


set_target_properties(slsMultiReceiver PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS slsMultiReceiver DESTINATION bin)
