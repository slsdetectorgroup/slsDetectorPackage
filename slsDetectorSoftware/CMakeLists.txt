set(SOURCES
    multiSlsDetector/multiSlsDetector.cpp 
    sharedMemory/SharedMemory.cpp
    slsDetector/slsDetectorUsers.cpp 
    slsDetector/slsDetectorCommand.cpp
    slsDetector/slsDetector.cpp 
)

set(HEADERS
)

add_library(slsDetectorShared SHARED
    ${SOURCES}
    ${HEADERS}
)
target_include_directories(slsDetectorShared PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/multiSlsDetector>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/sharedMemory>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/slsDetector>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(slsDetectorShared PUBLIC
    slsSupportLib
	${ZeroMQ_LIBRARIES}
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
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
