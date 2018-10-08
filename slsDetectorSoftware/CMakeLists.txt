set(SOURCES
    multiSlsDetector/multiSlsDetector.cpp 
    sharedMemory/SharedMemory.cpp
    slsDetector/slsDetectorUsers.cpp 
    slsDetector/slsDetectorCommand.cpp
    slsDetector/slsDetector.cpp 
    slsReceiverInterface/receiverInterface.cpp
    ../slsSupportLib/include/utilities.cpp
)

set(HEADERS
)

include_directories(
../slsSupportLib/include
multiSlsDetector 
sharedMemory
slsDetector 
slsReceiverInterface 
../slsSupportLib/include
)

add_library(slsDetectorStatic STATIC
    ${SOURCES}
    ${HEADERS}
)
set_target_properties(slsDetectorStatic PROPERTIES
    ARCHIVE_OUTPUT_NAME SlsDetector
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_library(slsDetectorShared SHARED
    ${SOURCES}
    ${HEADERS}
)

add_library(zmq STATIC IMPORTED GLOBAL)

set(ZMQ_STATIC_ARCHIVE ${CMAKE_CURRENT_SOURCE_DIR}/../slsSupportLib/include/libzmq.a)
set_target_properties(zmq PROPERTIES
    IMPORTED_LOCATION ${ZMQ_STATIC_ARCHIVE}
)

target_link_libraries(slsDetectorShared
	zmq
)

set(PUBLICHEADERS
    ../slsSupportLib/include/sls_detector_defs.h
    ../slsSupportLib/include/sls_detector_funcs.h
    ../slsSupportLib/include/error_defs.h
    ../slsSupportLib/include/versionAPI.h
    ../slsSupportLib/include/sls_receiver_exceptions.h
    ../slsSupportLib/include/utilties.h
    ../slsSupportLib/include/container_utils.h
    sharedMemory/SharedMemory.h
    slsDetector/slsDetector.h
    slsDetector/slsDetectorBase.h
    slsDetector/slsDetectorUsers.h
    slsDetector/detectorData.h
    multiSlsDetector/multiSlsDetector.h
    slsReceiverInterface/receiverInterface.h
)
set_target_properties(slsDetectorShared PROPERTIES
    LIBRARY_OUTPUT_NAME SlsDetector
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    PUBLIC_HEADER "${PUBLICHEADERS}"
)



add_subdirectory(slsDetectorClient)

if(DOXYGEN_FOUND)
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/slsDetectorUsers.doxy
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif()


install(FILES ${ZMQ_STATIC_ARCHIVE}
DESTINATION lib) 

install(TARGETS slsDetectorShared slsDetectorStatic
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include
    ARCHIVE DESTINATION lib)
