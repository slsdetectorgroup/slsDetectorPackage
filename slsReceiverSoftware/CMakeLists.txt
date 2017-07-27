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


if (REST)
	MESSAGE("\n *** Using REST Implementation *** \n")
  list (APPEND SOURCES src/UDPRestImplementation.cpp)
# add_library(poco etc.)
else (REST)
	MESSAGE("\n *** Using Standard Implementation *** \n")
endif (REST)



# HDF5
if (HDF5_FOUND)
	set (HDF5DIR '/opt/hdf5v1.10.0/')
	
	include_directories( 
		${HDF5DIR}/include 
	)
	add_definitions( 
		-DHDF5C 
	)
	add_library(hdf5 STATIC IMPORTED)
	add_library(hdf5_cpp STATIC IMPORTED)
	add_library(sz SHARED IMPORTED)
	add_library(z SHARED IMPORTED)
	set_target_properties(hdf5 PROPERTIES
		IMPORTED_LOCATION ${HDF5DIR}/lib/libhdf5.a
	)
	set_target_properties(hdf5_cpp PROPERTIES
		IMPORTED_LOCATION ${HDF5DIR}/lib/libhdf5_cpp.a
	)
	set_target_properties(sz PROPERTIES
		IMPORTED_LOCATION ${HDF5DIR}/lib/libsz.so
	)
	set_target_properties(z PROPERTIES
		IMPORTED_LOCATION ${HDF5DIR}/lib/libz.so
	)
	set(_hdf5_libs hdf5 hdf5_cpp sz z)
endif ()


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
    ${_hdf5_libs}
)
