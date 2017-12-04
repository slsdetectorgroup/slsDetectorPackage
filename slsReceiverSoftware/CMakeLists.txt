set(SOURCES
    src/MySocketTCP.cpp
    src/UDPInterface.cpp
    src/UDPBaseImplementation.cpp
    src/UDPStandardImplementation.cpp
    src/slsReceiverTCPIPInterface.cpp
    src/slsReceiver.cpp
    src/slsReceiverUsers.cpp
    src/utilities.cpp
    src/File.cpp
    src/BinaryFile.cpp
    src/ThreadObject.cpp
    src/Listener.cpp
    src/DataProcessor.cpp
    src/DataStreamer.cpp
    src/Fifo.cpp
)


# REST
if (REST)
    MESSAGE("\n *** Using REST Implementation *** \n")
    list (APPEND SOURCES src/UDPRestImplementation.cpp)
    # add_library(poco etc.)
else (REST)
    MESSAGE("\n *** Using Standard Implementation *** \n")
endif (REST)


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
	        src/HDF5FileStatic.cpp
	    )
	endif ()
endif (USE_HDF5)


add_definitions(
    -DDACS_INT  -DSLS_RECEIVER_UDP_FUNCTIONS
)

include_directories(
    include
)

add_library(zmq STATIC IMPORTED )

set_target_properties(zmq PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/include/libzmq.a
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
set_target_properties(slsReceiverShared PROPERTIES
    LIBRARY_OUTPUT_NAME SlsReceiver
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
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
