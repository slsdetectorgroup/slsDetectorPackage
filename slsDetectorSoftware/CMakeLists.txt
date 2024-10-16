# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
set(SOURCES
    src/DetectorImpl.cpp 
    src/Module.cpp 
    src/Detector.cpp
    src/HelpDacs.cpp
    src/CmdParser.cpp
    src/Pattern.cpp
    src/CtbConfig.cpp
    src/Caller.cpp
    src/CallerSpecial.cpp
    src/inferAction.cpp
)

add_library(slsDetectorObject OBJECT
    ${SOURCES}
)

target_include_directories(slsDetectorObject PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    PRIVATE
    ${SLS_INTERNAL_RAPIDJSON_DIR}
)

target_link_libraries(slsDetectorObject 
    PUBLIC
        slsProjectOptions
        slsSupportStatic
        pthread
        rt
    PRIVATE
        slsProjectWarnings
)

set(DETECTOR_LIBRARY_TARGETS slsDetectorObject)


set(PUBLICHEADERS
    include/sls/detectorData.h
    include/sls/Detector.h
    include/sls/Result.h
    include/sls/Pattern.h
)

#Shared library
if(SLS_BUILD_SHARED_LIBRARIES)
    add_library(slsDetectorShared SHARED $<TARGET_OBJECTS:slsDetectorObject>)
    target_link_libraries(slsDetectorShared PUBLIC slsDetectorObject)
    set_target_properties(slsDetectorShared PROPERTIES
        VERSION ${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}.${PACKAGE_VERSION_PATCH}
        SOVERSION ${PACKAGE_VERSION_MAJOR}
        LIBRARY_OUTPUT_NAME SlsDetector
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        PUBLIC_HEADER "${PUBLICHEADERS}"
    )
    list(APPEND DETECTOR_LIBRARY_TARGETS slsDetectorShared)
endif(SLS_BUILD_SHARED_LIBRARIES)

#Static library
add_library(slsDetectorStatic STATIC $<TARGET_OBJECTS:slsDetectorObject>)
target_link_libraries(slsDetectorStatic PUBLIC slsDetectorObject)

set_target_properties(slsDetectorStatic PROPERTIES
    ARCHIVE_OUTPUT_NAME SlsDetectorStatic
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    PUBLIC_HEADER "${PUBLICHEADERS}"
)
list(APPEND DETECTOR_LIBRARY_TARGETS slsDetectorStatic)

    



if((CMAKE_BUILD_TYPE STREQUAL "Release") AND SLS_LTO_AVAILABLE)
    set_property(TARGET ${DETECTOR_LIBRARY_TARGETS} PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif()


if(SLS_USE_TEXTCLIENT)
    set(det_bin_names "sls_detector_put" 
            "sls_detector_get" 
            "sls_detector_acquire" 
            "sls_detector_acquire_zmq"
            "sls_detector_help"
            "sls_detector"
            )
    set(det_cmd_name "PUT" "GET" "READOUT" "READOUTZMQ" "HELP" "INFER")
    list(LENGTH det_bin_names len1)
    math(EXPR len2 "${len1} - 1")

    foreach(val RANGE ${len2})
        list(GET det_bin_names ${val} val1)
        list(GET det_cmd_name ${val} val2)
        message(STATUS "${val1}  ${val2}")

        add_executable(${val1} src/CmdApp.cpp)

        target_link_libraries(${val1} 
        slsDetectorStatic
        pthread
        rt
        )
        SET_SOURCE_FILES_PROPERTIES( src/Caller.cpp PROPERTIES COMPILE_FLAGS "-Wno-unused-variable -Wno-unused-but-set-variable")

        set_target_properties(${val1} PROPERTIES 
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
            COMPILE_DEFINITIONS ${val2}=1
        )
        if((CMAKE_BUILD_TYPE STREQUAL "Release") AND SLS_LTO_AVAILABLE)
            set_property(TARGET ${val1} PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
        endif()
    endforeach()
    install(TARGETS ${det_bin_names} DESTINATION bin)


endif(SLS_USE_TEXTCLIENT)

if (SLS_USE_TESTS)
    add_subdirectory(tests)
endif(SLS_USE_TESTS)


install(TARGETS ${DETECTOR_LIBRARY_TARGETS}
    EXPORT "${TARGETS_EXPORT_NAME}"
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sls
)
