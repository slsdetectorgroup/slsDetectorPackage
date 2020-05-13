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

set(HEADERS
)

set(PUBLICHEADERS
    include/sls_detector_defs.h
    include/sls_detector_funcs.h
    include/versionAPI.h
    include/sls_detector_exceptions.h
    include/file_utils.h
    include/container_utils.h
    include/string_utils.h
    include/ClientSocket.h
    include/DataSocket.h
    include/ServerSocket.h
    include/ServerInterface.h
    include/network_utils.h
    include/FixedCapacityContainer.h
    include/ToString.h
    include/TimeHelper.h
    include/TypeTraits.h
    include/Timer.h
    include/UdpRxSocket.h
)

add_library(slsSupportLib SHARED
    ${SOURCES}
    ${HEADERS}
)


if(SLS_LTO_AVAILABLE)
    set_property(TARGET slsSupportLib PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif()



target_include_directories(slsSupportLib PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_include_directories(slsSupportLib PRIVATE
    ${ZeroMQ_INCLUDE_DIRS}
)


set_target_properties(slsSupportLib PROPERTIES
    LIBRARY_OUTPUT_NAME SlsSupport
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    PUBLIC_HEADER "${PUBLICHEADERS}"
)

target_link_libraries(slsSupportLib 
    PUBLIC
    slsProjectOptions 
    # ${ZeroMQ_LIBRARIES}
    zmq
    PRIVATE
    
    slsProjectWarnings 
    PUBLIC
    rapidjson
)



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
