set(SOURCES
    src/Implementation.cpp
    src/ClientInterface.cpp
    src/Receiver.cpp
    src/File.cpp
    src/BinaryFile.cpp
    src/ThreadObject.cpp
    src/Listener.cpp
    src/DataProcessor.cpp
    src/DataStreamer.cpp
    src/Fifo.cpp
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
	        src/HDF5File.cpp 
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


#Shared library
add_library(slsReceiverShared SHARED $<TARGET_OBJECTS:slsReceiverObject>)
target_link_libraries(slsReceiverShared PUBLIC slsReceiverObject)
set_target_properties(slsReceiverShared PROPERTIES
    LIBRARY_OUTPUT_NAME SlsReceiver
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    PUBLIC_HEADER "${PUBLICHEADERS}"
)

#Static library 
add_library(slsReceiverStatic STATIC $<TARGET_OBJECTS:slsReceiverObject>)
target_link_libraries(slsReceiverStatic PUBLIC slsReceiverObject)
set_target_properties(slsReceiverStatic PROPERTIES
    ARCHIVE_OUTPUT_NAME SlsReceiverStatic
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    PUBLIC_HEADER "${PUBLICHEADERS}"
)

# message("Receiver LTO_AVAILABLE:" ${SLS_LTO_AVAILABLE})
if((CMAKE_BUILD_TYPE STREQUAL "Release") AND SLS_LTO_AVAILABLE)
    set_property(TARGET slsReceiverObject PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
    set_property(TARGET slsReceiverStatic PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
    set_property(TARGET slsReceiverShared PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif()


add_executable(slsReceiver
    src/ReceiverApp.cpp
)


set_target_properties(slsReceiver PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
if(SLS_LTO_AVAILABLE)
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
if(SLS_LTO_AVAILABLE)
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

if (SLS_USE_TESTS)
    add_subdirectory(tests)
endif(SLS_USE_TESTS)


install(TARGETS slsReceiverShared slsReceiver slsMultiReceiver slsReceiverObject slsReceiverStatic
        EXPORT "${TARGETS_EXPORT_NAME}"
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sls
)

