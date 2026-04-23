set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TCPInterface.cpp 
)

add_library(slsServerObject OBJECT
    ${SOURCES}
)

target_include_directories(slsServerObject PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(slsServerObject 
    PUBLIC
        slsProjectOptions
        slsSupportStatic
    PRIVATE
        slsProjectWarnings
)

set(DETECTOR_LIBRARY_TARGETS slsServerObject)

set(PUBLICHEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/TCPInterface.h
)

#Shared library
if(SLS_BUILD_SHARED_LIBRARIES)
    add_library(slsServerShared SHARED $<TARGET_OBJECTS:slsServerObject>)
    target_link_libraries(slsServerShared PUBLIC slsServerObject)
    set_target_properties(slsServerShared PROPERTIES
        VERSION ${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}.${PACKAGE_VERSION_PATCH}
        SOVERSION ${PACKAGE_VERSION_MAJOR}
        LIBRARY_OUTPUT_NAME SlsServer
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        PUBLIC_HEADER "${PUBLICHEADERS}"
    )
    list(APPEND DETECTOR_LIBRARY_TARGETS slsServerShared)
endif(SLS_BUILD_SHARED_LIBRARIES)

#Static library
add_library(slsServerStatic STATIC $<TARGET_OBJECTS:slsServerObject>)
target_link_libraries(slsServerStatic PUBLIC slsServerObject)

set_target_properties(slsServerStatic PROPERTIES
    ARCHIVE_OUTPUT_NAME SlsServerStatic
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    PUBLIC_HEADER "${PUBLICHEADERS}"
)
list(APPEND DETECTOR_LIBRARY_TARGETS slsServerStatic)

if((CMAKE_BUILD_TYPE STREQUAL "Release") AND SLS_LTO_AVAILABLE)
    set_property(TARGET ${DETECTOR_LIBRARY_TARGETS} PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif()

install(TARGETS ${DETECTOR_LIBRARY_TARGETS}
    EXPORT "${TARGETS_EXPORT_NAME}"
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sls
)
