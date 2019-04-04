set(SOURCES
    src/multiSlsDetector.cpp 
    src/slsDetectorUsers.cpp 
    src/slsDetectorCommand.cpp
    src/slsDetector.cpp 
)

set(HEADERS
)

add_library(slsDetectorShared SHARED
    ${SOURCES}
    ${HEADERS}
)
target_include_directories(slsDetectorShared PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(slsDetectorShared PUBLIC
    slsProjectOptions
    slsProjectWarnings
    slsSupportLib
	${ZeroMQ_LIBRARIES}
)

set(PUBLICHEADERS
    include/SharedMemory.h
    include/slsDetector.h
    include/slsDetectorUsers.h
    include/detectorData.h
    include/multiSlsDetector.h
    include/gitInfoLib.h
)
set_target_properties(slsDetectorShared PROPERTIES
    LIBRARY_OUTPUT_NAME SlsDetector
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    PUBLIC_HEADER "${PUBLICHEADERS}"
)

# add_subdirectory(slsDetectorClient)

add_executable(sls_detector_get
    src/sls_detector_client.cpp
)
target_link_libraries(sls_detector_get
   slsProjectOptions
   slsProjectWarnings
   slsDetectorShared
   slsSupportLib
   pthread
   ${ZeroMQ_LIBRARIES}
   rt
)
set_target_properties(sls_detector_get PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMPILE_DEFINITIONS GET=1
)

add_executable(sls_detector_put
    src/sls_detector_client.cpp
)
target_link_libraries(sls_detector_put
   slsProjectOptions
   slsProjectWarnings
   slsDetectorShared
   pthread
   ${ZeroMQ_LIBRARIES}
   rt
)
set_target_properties(sls_detector_put PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMPILE_DEFINITIONS PUT=1
)

add_executable(sls_detector_acquire
    src/sls_detector_client.cpp
)
target_link_libraries(sls_detector_acquire
   slsProjectOptions
   slsProjectWarnings
   slsDetectorShared
   pthread
   ${ZeroMQ_LIBRARIES}
   rt
)
set_target_properties(sls_detector_acquire PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMPILE_DEFINITIONS READOUT=1
)

add_executable(sls_detector_help
    src/sls_detector_client.cpp
)
target_link_libraries(sls_detector_help
   slsProjectOptions
   slsProjectWarnings
   slsDetectorShared
   pthread
   ${ZeroMQ_LIBRARIES}
   rt
)
set_target_properties(sls_detector_help PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMPILE_DEFINITIONS HELP=1
)

install(TARGETS sls_detector_put sls_detector_get sls_detector_acquire sls_detector_help DESTINATION bin)


if(DOXYGEN_FOUND)
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/slsDetectorUsers.doxy
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif()

if (SLS_USE_TESTS)
    add_subdirectory(tests)
endif(SLS_USE_TESTS)


install(TARGETS slsDetectorShared
    EXPORT "${TARGETS_EXPORT_NAME}"
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
