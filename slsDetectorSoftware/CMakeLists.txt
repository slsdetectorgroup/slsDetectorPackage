set(SOURCES
    multiSlsDetector/multiSlsDetector.cpp 
    slsDetector/slsDetectorUsers.cpp 
    slsDetector/slsDetectorUtils.cpp
    slsDetector/slsDetectorCommand.cpp
    slsDetector/slsDetectorActions.cpp
    slsDetector/slsDetector.cpp 
    slsDetectorAnalysis/angularConversion.cpp 
    slsDetectorAnalysis/angularConversionStatic.cpp 
    slsDetectorAnalysis/energyConversion.cpp 
    slsDetectorAnalysis/fileIO.cpp
    slsDetectorAnalysis/postProcessing.cpp 
    slsDetectorAnalysis/postProcessingFuncs.cpp
    slsReceiverInterface/receiverInterface.cpp
    threadFiles/CondVar.cpp  
    threadFiles/Mutex.cpp 
    threadFiles/ThreadPool.cpp
    usersFunctions/usersFunctions.cpp 
)

set(HEADERS
)

include_directories(
commonFiles
multiSlsDetector 
slsDetector 
slsDetectorUtils 
slsDetectorCommand 
slsDetectorAnalysis 
slsReceiverInterface 
threadFiles
usersFunctions 
../slsReceiverSoftware/include 
../slsReceiverSoftware/MySocketTCP 
)

add_definitions(
    -DDACS_INT
)

add_library(zmq STATIC IMPORTED )

set_target_properties(zmq PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../slsReceiverSoftware/include/libzmq.a
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
set_target_properties(slsDetectorShared PROPERTIES
    LIBRARY_OUTPUT_NAME SlsDetector
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
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

