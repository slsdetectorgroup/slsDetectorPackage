set(SOURCES
    src/ClientInterface.cpp
    src/CmdLineParser.cpp
    src/string_utils.cpp
    src/file_utils.cpp
    src/ClientSocket.cpp
    src/DataSocket.cpp
    src/ServerSocket.cpp
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
)

add_library(slsSupportLib SHARED
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(slsSupportLib PUBLIC include)

set_target_properties(slsSupportLib PROPERTIES
    LIBRARY_OUTPUT_NAME SlsSupport
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    PUBLIC_HEADER "${PUBLICHEADERS}"
)

if (USE_TESTS)
    add_subdirectory(tests)
endif(USE_TESTS)

install(TARGETS slsSupportLib
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include
    ARCHIVE DESTINATION lib
)
