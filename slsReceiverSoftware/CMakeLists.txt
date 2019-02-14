set(SOURCES
    src/slsReceiverImplementation.cpp
    src/slsReceiverTCPIPInterface.cpp
    src/slsReceiver.cpp
    src/slsReceiverUsers.cpp
    src/File.cpp
    src/BinaryFile.cpp
    src/ThreadObject.cpp
    src/Listener.cpp
    src/DataProcessor.cpp
    src/DataStreamer.cpp
    src/Fifo.cpp
)


# HDF5
if (SLS_USE_HDF5)
	if (HDF5_FOUND)
	    include_directories( 
	        ${HDF5_INCLUDE_DIRS}
	    )
	    add_definitions( 
	        -DHDF5C ${HDF5_DEFINITIONS}
	    )
	    list (APPEND SOURCES 
	        src/HDF5File.cpp 
	    )
	endif ()
endif (SLS_USE_HDF5)


# include_directories(
#     ${SLS_SUPPORT_LIB_INCLUDE_DIRS}
# )

add_library(slsReceiverShared SHARED
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(slsReceiverShared PUBLIC
    slsSupportLib
)

target_include_directories(slsReceiverShared PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

#What is included in slsReceiverLib?
set(PUBLICHEADERS
    include/slsReceiverUsers.h
)


set_target_properties(slsReceiverShared PROPERTIES
    LIBRARY_OUTPUT_NAME SlsReceiver
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    PUBLIC_HEADER "${PUBLICHEADERS}"
)

add_executable(slsReceiver
    src/main.cpp
)
set_target_properties(slsReceiver PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

target_link_libraries(slsReceiver PUBLIC
    slsSupportLib
    slsReceiverShared
    pthread
    zmq
    rt
)

if (HDF5_FOUND)
    target_link_libraries(slsReceiver PUBLIC ${HDF5_LIBRARIES})
endif ()



install(TARGETS slsReceiverShared
        EXPORT "${TARGETS_EXPORT_NAME}"
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES ${ZMQ_STATIC_ARCHIVE}
DESTINATION lib)
