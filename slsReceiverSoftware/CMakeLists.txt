# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
set(SOURCES
    src/Implementation.cpp
    src/ClientInterface.cpp
    src/Receiver.cpp
    src/BinaryDataFile.cpp
    src/ThreadObject.cpp
    src/Listener.cpp
    src/DataProcessor.cpp
    src/DataStreamer.cpp
    src/Fifo.cpp
    src/Arping.cpp
    src/MasterAttributes.cpp
    src/MasterFileUtility.cpp
)

set(PUBLICHEADERS
    include/sls/Receiver.h
)

# HDF5 file writing 
if (SLS_USE_HDF5)
    find_package(HDF5 1.10 COMPONENTS CXX REQUIRED)
	    add_definitions( 
	        -DHDF5C ${HDF5_DEFINITIONS}
	    )
	    list (APPEND SOURCES 
	        src/HDF5DataFile.cpp 
	    )
endif (SLS_USE_HDF5)

# Create an object library to avoid building the library twice
# This is only used during the build phase
add_library(slsReceiverObject OBJECT
    ${SOURCES}
)

target_include_directories(slsReceiverObject PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    PRIVATE
    ${SLS_INTERNAL_RAPIDJSON_DIR}
)

target_link_libraries(slsReceiverObject 
  PUBLIC
    slsProjectOptions
    slsSupportStatic
  PRIVATE
    slsProjectWarnings #don't propagate warnigns 
)

# HDF5
if (SLS_USE_HDF5)
    if (HDF5_FOUND)
        target_link_libraries(slsReceiverObject PUBLIC 
            ${HDF5_LIBRARIES})
        target_include_directories(slsReceiverObject PUBLIC
        	${HDF5_INCLUDE_DIRS}
            ${CMAKE_INSTALL_PREFIX}/include)
    endif ()
endif (SLS_USE_HDF5)

set(RECEIVER_LIBRARY_TARGETS slsReceiverObject)

#Shared library
if(SLS_BUILD_SHARED_LIBRARIES)
    add_library(slsReceiverShared SHARED $<TARGET_OBJECTS:slsReceiverObject>)
    target_link_libraries(slsReceiverShared PUBLIC slsReceiverObject)
    set_target_properties(slsReceiverShared PROPERTIES
        VERSION ${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}.${PACKAGE_VERSION_PATCH}
        SOVERSION ${PACKAGE_VERSION_MAJOR}
        LIBRARY_OUTPUT_NAME SlsReceiver
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        PUBLIC_HEADER "${PUBLICHEADERS}"
    )
    list(APPEND RECEIVER_LIBRARY_TARGETS slsReceiverShared)
endif(SLS_BUILD_SHARED_LIBRARIES)

#Static library 
add_library(slsReceiverStatic STATIC $<TARGET_OBJECTS:slsReceiverObject>)
target_link_libraries(slsReceiverStatic PUBLIC slsReceiverObject)
set_target_properties(slsReceiverStatic PROPERTIES
    ARCHIVE_OUTPUT_NAME SlsReceiverStatic
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    PUBLIC_HEADER "${PUBLICHEADERS}"
)
list(APPEND RECEIVER_LIBRARY_TARGETS slsReceiverStatic)


if((CMAKE_BUILD_TYPE STREQUAL "Release") AND SLS_LTO_AVAILABLE)
    set_property(TARGET ${RECEIVER_LIBRARY_TARGETS} PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif()



# Receiver binaries 

if (SLS_USE_RECEIVER_BINARIES)
    add_executable(slsReceiver
        src/ReceiverApp.cpp
    )

    set_target_properties(slsReceiver PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    if((CMAKE_BUILD_TYPE STREQUAL "Release") AND SLS_LTO_AVAILABLE)
        set_property(TARGET slsReceiver PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
    endif()

    target_link_libraries(slsReceiver PUBLIC
    PUBLIC
        slsReceiverStatic
        pthread
        rt
    PRIVATE
        slsProjectWarnings
    )

    add_executable(slsMultiReceiver
        src/MultiReceiverApp.cpp
    )

    set_target_properties(slsMultiReceiver PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    if((CMAKE_BUILD_TYPE STREQUAL "Release") AND SLS_LTO_AVAILABLE)
        set_property(TARGET slsMultiReceiver PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
    endif()

    target_link_libraries(slsMultiReceiver 
    PUBLIC
        slsReceiverStatic
        pthread
        rt
    PRIVATE
        slsProjectWarnings
    )

    add_executable(slsFrameSynchronizer
        src/FrameSynchronizerApp.cpp
    )

    set_target_properties(slsFrameSynchronizer PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    if((CMAKE_BUILD_TYPE STREQUAL "Release") AND SLS_LTO_AVAILABLE)
        set_property(TARGET slsFrameSynchronizer PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
    endif()

    target_link_libraries(slsFrameSynchronizer 
    PUBLIC
        slsReceiverStatic
        pthread
        rt
    PRIVATE
        slsProjectWarnings
        "$<BUILD_INTERFACE:libzmq-static>"
    )

    install(TARGETS slsReceiver slsMultiReceiver slsFrameSynchronizer
        EXPORT "${TARGETS_EXPORT_NAME}"
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sls
    )

endif(SLS_USE_RECEIVER_BINARIES)

if (SLS_USE_TESTS)
    add_subdirectory(tests)
endif(SLS_USE_TESTS)


install(TARGETS ${RECEIVER_LIBRARY_TARGETS}
        EXPORT "${TARGETS_EXPORT_NAME}"
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sls
)

