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
    ../slsSupportLib/include/utilities.cpp
    ../slsSupportLib/include/ClientInterface.cpp
)


# HDF5
if (USE_HDF5)
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
endif (USE_HDF5)


add_definitions(
    -DDACS_INT  -DSLS_RECEIVER_UDP_FUNCTIONS
)

include_directories(
    include
    ../slsSupportLib/include
)


add_library(slsReceiverStatic STATIC
    ${SOURCES}
    ${HEADERS}
)
set_target_properties(slsReceiverStatic PROPERTIES
    ARCHIVE_OUTPUT_NAME SlsReceiver
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_library(slsReceiverShared SHARED
    ${SOURCES}
    ${HEADERS}
)

set(PUBLICHEADERS
    ../slsSupportLib/include/sls_detector_defs.h
    ../slsSupportLib/include/ansi.h
    ../slsSupportLib/include/sls_detector_funcs.h
    ../slsSupportLib/include/MySocketTCP.h
    ../slsSupportLib/include/genericSocket.h
    ../slsSupportLib/include/logger.h
	../slsSupportLib/include/sls_detector_exceptions.h
	../slsSupportLib/include/utilities.h
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
target_link_libraries(slsReceiver
    slsReceiverShared
    pthread
    zmq
    rt
)

if (HDF5_FOUND)
    target_link_libraries(slsReceiver ${HDF5_LIBRARIES})
endif ()



install(TARGETS slsReceiverShared slsReceiverStatic slsReceiver
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include)
        
      