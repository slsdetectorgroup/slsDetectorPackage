set(SOURCES
    multiSlsDetector/multiSlsDetector.cpp 
    sharedMemory/SharedMemory.cpp
    slsDetector/slsDetectorUsers.cpp 
    slsDetector/slsDetectorCommand.cpp
    slsDetector/slsDetector.cpp 
    ../slsSupportLib/include/ClientInterface.cpp
    ../slsSupportLib/include/utilities.cpp
    ../slsSupportLib/src/string_utils.cpp
)

set(HEADERS
)

include_directories(
../slsSupportLib/include
multiSlsDetector 
sharedMemory
slsDetector 
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

target_link_libraries(slsDetectorShared
	zmq
)



set(PUBLICHEADERS
    ${PROJECT_SOURCE_DIR}/slsSupportLib/include/sls_detector_defs.h
    ${PROJECT_SOURCE_DIR}/slsSupportLib/include/sls_detector_funcs.h
    ${PROJECT_SOURCE_DIR}/slsSupportLib/include/error_defs.h
    ${PROJECT_SOURCE_DIR}/slsSupportLib/include/versionAPI.h
    ${PROJECT_SOURCE_DIR}/slsSupportLib/include/sls_detector_exceptions.h
    ${PROJECT_SOURCE_DIR}/slsSupportLib/include/utilities.h
    ${PROJECT_SOURCE_DIR}/slsSupportLib/include/container_utils.h
    ${PROJECT_SOURCE_DIR}/slsSupportLib/include/string_utils.h
    sharedMemory/SharedMemory.h
    slsDetector/slsDetector.h
    slsDetector/slsDetectorUsers.h
    slsDetector/detectorData.h
    multiSlsDetector/multiSlsDetector.h
    ${PROJECT_SOURCE_DIR}/slsSupportLib/include/ClientInterface.h
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


install(TARGETS slsDetectorShared slsDetectorStatic
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include
    ARCHIVE DESTINATION lib)
