set(SOURCES
    src/ClientInterface.cpp
    src/CmdLineParser.cpp
    src/string_utils.cpp
    src/file_utils.cpp
    src/ClientSocket.cpp
    src/DataSocket.cpp
    src/ServerSocket.cpp
    src/ServerInterface.cpp
    src/network_utils.cpp
)

set(HEADERS
)

set(PUBLICHEADERS
    include/ansi.h
    include/sls_detector_defs.h
    include/sls_detector_funcs.h
    include/error_defs.h
    include/versionAPI.h
    include/sls_detector_exceptions.h
    include/file_utils.h
    include/container_utils.h
    include/string_utils.h
    include/ClientInterface.h
    include/MySocketTCP.h
    include/genericSocket.h
    include/logger.h
    include/ClientSocket.h
    include/DataSocket.h
    include/ServerSocket.h
    include/ServerInterface.h
    include/network_utils.h
)

add_library(slsSupportLib SHARED
    ${SOURCES}
    ${HEADERS}
)

check_ipo_supported(RESULT result)
if(result)
    set_property(TARGET slsSupportLib PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif()

target_include_directories(slsSupportLib PUBLIC
    ${ZeroMQ_INCLUDE_DIRS}
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

set_target_properties(slsSupportLib PROPERTIES
    LIBRARY_OUTPUT_NAME SlsSupport
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    PUBLIC_HEADER "${PUBLICHEADERS}"
)

target_link_libraries(slsSupportLib slsProjectOptions slsProjectWarnings)

if (SLS_USE_TESTS)
    add_subdirectory(tests)
endif(SLS_USE_TESTS)

# Install the library
install(TARGETS slsSupportLib
    EXPORT "${TARGETS_EXPORT_NAME}"
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
