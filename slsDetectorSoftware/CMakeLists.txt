set(SOURCES
    src/DetectorImpl.cpp 
    src/slsDetectorUsers.cpp 
    src/Module.cpp 
    src/Detector.cpp
    src/CmdProxy.cpp
    src/CmdParser.cpp
)

set(HEADERS
)

add_library(slsDetectorShared SHARED
    ${SOURCES}
    ${HEADERS}
)

# Do we have link time optimization?
check_ipo_supported(RESULT LTO_AVAILABLE)
if(LTO_AVAILABLE)
    set_property(TARGET slsDetectorShared PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif()


target_include_directories(slsDetectorShared PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(slsDetectorShared 
    PUBLIC
    slsSupportLib
    pthread
    rt
    slsProjectOptions
    PRIVATE
    slsProjectWarnings
    ${ZeroMQ_LIBRARIES}
)


set(PUBLICHEADERS
    include/slsDetectorUsers.h
    include/detectorData.h
    include/Detector.h
    include/Result.h
)
set_target_properties(slsDetectorShared PROPERTIES
    LIBRARY_OUTPUT_NAME SlsDetector
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    PUBLIC_HEADER "${PUBLICHEADERS}"
)



# Loop over list to generate command line binaries
set(bin_names "sls_detector_put" 
          "sls_detector_get" 
          "sls_detector_acquire" 
          "sls_detector_help")
set(cmd_name "PUT" "GET" "READOUT" "HELP")
list(LENGTH bin_names len1)
math(EXPR len2 "${len1} - 1")

foreach(val RANGE ${len2})
    list(GET bin_names ${val} val1)
    list(GET cmd_name ${val} val2)
    message(STATUS "${val1}  ${val2}")

    add_executable(${val1} src/CmdLineApp.cpp)
    target_link_libraries(${val1} 
        slsDetectorShared
        pthread
        zmq
        rt
    )
    set_target_properties(${val1} PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        COMPILE_DEFINITIONS ${val2}=1
    )
    if(LTO_AVAILABLE)
        set_property(TARGET ${val1} PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
    endif()
endforeach()


install(TARGETS ${bin_names} DESTINATION bin)


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
