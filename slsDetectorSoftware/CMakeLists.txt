set(SOURCES
    multiSlsDetector/multiSlsDetector.cpp 
    sharedMemory/SharedMemory.cpp
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
    sharedMemory
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

add_library(slsDetectorShared SHARED
    ${SOURCES}
    ${HEADERS}
)

set(PUBLICHEADERS
    commonFiles/sls_detector_defs.h
    commonFiles/sls_detector_funcs.h
    commonFiles/error_defs.h
    commonFiles/versionAPI.h
    sharedMemory/SharedMemory.h
    slsDetector/slsDetectorUtils.h
    slsDetector/slsDetector.h
    slsDetector/slsDetectorActions.h
    slsDetector/slsDetectorBase.h
    slsDetector/slsDetectorUsers.h
    slsDetectorAnalysis/postProcessing.h
    slsDetectorAnalysis/detectorData.h
    slsDetectorAnalysis/angularConversion.h
    slsDetectorAnalysis/angularConversionStatic.h
    slsDetectorAnalysis/angleConversionConstant.h
    slsDetectorAnalysis/badChannelCorrections.h
    slsDetectorAnalysis/energyConversion.h
    slsDetectorAnalysis/fileIO.h
    slsDetectorAnalysis/fileIOStatic.h
    multiSlsDetector/multiSlsDetector.h
    slsReceiverInterface/receiverInterface.h
    ../slsReceiverSoftware/include/sls_receiver_exceptions.h
    slsDetector/slsDetectorUsers.h
    ../slsReceiverSoftware/include/slsReceiverUsers.h
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

install(TARGETS slsDetectorShared
    EXPORT "${TARGETS_EXPORT_NAME}"
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include
    ARCHIVE DESTINATION lib)
