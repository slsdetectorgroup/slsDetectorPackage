set(SOURCES
    mainReceiver.cpp
)

include_directories(
    ../../slsReceiverSoftware/include
    ../../slsDetectorSoftware/slsDetectorAnalysis 
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
    ${HDF5_LIBRARIES}
)

if (HDF5_FOUND)
    target_link_libraries(slsMultiReceiver 
    ${HDF5_LIBRARIES}
    )
endif ()


set_target_properties(slsMultiReceiver PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS slsMultiReceiver DESTINATION bin)
