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

# Create a Object library to avoidbuilding the library twice
add_library(slsSupportObject OBJECT
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(slsSupportObject PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)


target_include_directories(slsSupportObject PRIVATE
    ${ZeroMQ_INCLUDE_DIRS}
)

target_link_libraries(slsSupportObject 
    PUBLIC
    slsProjectOptions 
    zmq
    rapidjson
    PRIVATE
    slsProjectWarnings     
)

if (SLS_USE_TESTS)
    add_subdirectory(tests)
endif(SLS_USE_TESTS)

add_library(slsSupportLib SHARED $<TARGET_OBJECTS:slsSupportObject>)
target_link_libraries(slsSupportLib PUBLIC slsSupportObject)
# target_include_directories(slsSupportLib PUBLIC
#     "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
#     "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
# )

set_target_properties(slsSupportLib PROPERTIES
    LIBRARY_OUTPUT_NAME SlsSupport
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    PUBLIC_HEADER "${PUBLICHEADERS}"
)

add_library(slsSupportLibStatic STATIC $<TARGET_OBJECTS:slsSupportObject>)
target_link_libraries(slsSupportLibStatic PUBLIC slsSupportObject)

set_target_properties(slsSupportLibStatic PROPERTIES
    ARCHIVE_OUTPUT_NAME SlsSupportStatic
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    PUBLIC_HEADER "${PUBLICHEADERS}"
)

# target_include_directories(slsSupportLibStatic PUBLIC
#     "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
#     "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
# )

if((CMAKE_BUILD_TYPE STREQUAL "Release") AND SLS_LTO_AVAILABLE)
    set_property(TARGET slsSupportObject PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
    set_property(TARGET slsSupportLib  PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
    set_property(TARGET slsSupportLibStatic PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif()

# Install the library
install(TARGETS slsSupportLib slsSupportLibStatic slsSupportObject
    EXPORT "${TARGETS_EXPORT_NAME}"
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

