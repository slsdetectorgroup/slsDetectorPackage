set(SOURCES
    sls_detector_client.cpp
)

include_directories(
    ../commonFiles
    ../multiSlsDetector 
    ../slsDetector 
    ../slsDetectorAnalysis 
    ../slsReceiverInterface 
    ../usersFunctions 
    ../MySocketTCP
)

add_executable(sls_detector_get
    ${SOURCES}
)
target_link_libraries(sls_detector_get
   slsDetectorShared
   pthread
   zmq
   rt
)
set_target_properties(sls_detector_get PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMPILE_DEFINITIONS GET=1
)

add_executable(sls_detector_put
    ${SOURCES}
)
target_link_libraries(sls_detector_put
   slsDetectorShared
   pthread
   zmq
   rt
)
set_target_properties(sls_detector_put PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMPILE_DEFINITIONS PUT=1
)

add_executable(sls_detector_acquire
    ${SOURCES}
)
target_link_libraries(sls_detector_acquire
   slsDetectorShared
   pthread
   zmq
   rt
)
set_target_properties(sls_detector_acquire PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMPILE_DEFINITIONS READOUT=1
)

add_executable(sls_detector_help
    ${SOURCES}
)
target_link_libraries(sls_detector_help
   slsDetectorShared
   pthread
   zmq
   rt
)
set_target_properties(sls_detector_help PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMPILE_DEFINITIONS HELP=1
)

