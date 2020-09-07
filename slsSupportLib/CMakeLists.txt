set(SOURCES
    src/string_utils.cpp
    src/file_utils.cpp
    src/ClientSocket.cpp
    src/DataSocket.cpp
    src/ServerSocket.cpp
    src/ServerInterface.cpp
    src/ToString.cpp
    src/network_utils.cpp
    src/ZmqSocket.cpp
    src/UdpRxSocket.cpp
    src/sls_detector_exceptions.cpp
)

# Header files to install as a part of the library
set(PUBLICHEADERS
    include/sls_detector_defs.h
    include/sls_detector_exceptions.h
    include/container_utils.h
    include/string_utils.h
    include/network_utils.h
    include/ToString.h
    include/TypeTraits.h
    include/TimeHelper.h
)

# Additional headers to be installed if SLS_DEVEL_HEADERS
# is specified as an option. Aim is to give the developer
# access to utilities in the support library while
# at the same time offer a minimal install
if(SLS_DEVEL_HEADERS)
    set(PUBLICHEADERS
        ${PUBLICHEADERS}
        include/file_utils.h
        include/sls_detector_funcs.h
        include/ClientSocket.h
        include/DataSocket.h
        include/ServerSocket.h
        include/ServerInterface.h
        include/Timer.h
        include/StaticVector.h
        include/UdpRxSocket.h
        include/versionAPI.h
        include/ZmqSocket.h
        include/bit_utils.h
    )
endif()

# Create an object library to avoid building the library twice
add_library(slsSupportObject OBJECT
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(slsSupportObject 
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  PRIVATE
    ${ZeroMQ_INCLUDE_DIRS}
)

target_link_libraries(slsSupportObject 
  PUBLIC
    slsProjectOptions 
    # zmq
    ${ZeroMQ_LIBRARIES}
    rapidjson
  PRIVATE
    slsProjectWarnings     
)

if (SLS_USE_TESTS)
    add_subdirectory(tests)
endif(SLS_USE_TESTS)

# Add shared library version of the support lib
add_library(slsSupportShared SHARED $<TARGET_OBJECTS:slsSupportObject>)

target_link_libraries(slsSupportShared PUBLIC slsSupportObject)

set_target_properties(slsSupportShared PROPERTIES
    LIBRARY_OUTPUT_NAME SlsSupport
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    PUBLIC_HEADER "${PUBLICHEADERS}"
)

# Add static version of the support lib
add_library(slsSupportStatic STATIC $<TARGET_OBJECTS:slsSupportObject>)

target_link_libraries(slsSupportStatic PUBLIC slsSupportObject)

set_target_properties(slsSupportStatic PROPERTIES
    ARCHIVE_OUTPUT_NAME SlsSupportStatic
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    PUBLIC_HEADER "${PUBLICHEADERS}"
)

if((CMAKE_BUILD_TYPE STREQUAL "Release") AND SLS_LTO_AVAILABLE)
    set_property(TARGET slsSupportObject PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
    set_property(TARGET slsSupportShared  PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
    set_property(TARGET slsSupportStatic PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif()

install(TARGETS slsSupportShared slsSupportStatic slsSupportObject
    EXPORT "${TARGETS_EXPORT_NAME}"
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

