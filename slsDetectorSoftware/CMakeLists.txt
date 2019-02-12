set(SOURCES
    multiSlsDetector/multiSlsDetector.cpp 
    sharedMemory/SharedMemory.cpp
    slsDetector/slsDetectorUsers.cpp 
    slsDetector/slsDetectorCommand.cpp
    slsDetector/slsDetector.cpp 
)

set(HEADERS
)

# include_directories(
#     multiSlsDetector 
#     sharedMemory
#     slsDetector 
# )

add_library(slsDetectorShared SHARED
    ${SOURCES}
    ${HEADERS}
)
target_include_directories(slsDetectorShared PUBLIC 
    multiSlsDetector 
    sharedMemory
    slsDetector 
)

target_link_libraries(slsDetectorShared
    slsSupportLib
	zmq
)

set(PUBLICHEADERS
    sharedMemory/SharedMemory.h
    slsDetector/slsDetector.h
    slsDetector/slsDetectorUsers.h
    slsDetector/detectorData.h
    multiSlsDetector/multiSlsDetector.h
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
    ARCHIVE DESTINATION lib
)
