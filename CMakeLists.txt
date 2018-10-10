cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set (CALIBRATE OFF)

option (USE_HDF5 "HDF5 File format" OFF)
option (USE_TEXTCLIENT "Text Client" OFF)
option (USE_RECEIVER "Receiver" OFF)
option (USE_GUI "GUI" OFF)


if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6.0)
	set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wno-misleading-indentation")
else ()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
endif ()

find_package(Qt4)
find_package(Qwt 6)
find_package(CBF)
find_package(Doxygen)
# find_package(PNG REQUIRED)

if (USE_HDF5)
	find_package(HDF5 1.10 COMPONENTS CXX)
endif (USE_HDF5)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_INSTALL_RPATH "$ORIGIN")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)


#zmq
add_library(zmq STATIC IMPORTED GLOBAL)
set(ZMQ_STATIC_ARCHIVE ${CMAKE_CURRENT_SOURCE_DIR}/slsSupportLib/include/libzmq.a)
set_target_properties(zmq PROPERTIES
    IMPORTED_LOCATION ${ZMQ_STATIC_ARCHIVE}
)


if (USE_TEXTCLIENT)
       add_subdirectory(slsDetectorSoftware)
endif (USE_TEXTCLIENT)

if (USE_RECEIVER)
       add_subdirectory(slsReceiverSoftware)
       add_subdirectory(manual/manual-api)    
endif (USE_RECEIVER)
               
               
if (USE_GUI)   
       if (QT4_FOUND AND QWT_FOUND)
       add_subdirectory(slsDetectorGui)
       endif()
endif (USE_GUI)

if (USE_SUPPORT_LIB)
    add_subdirectory(slsSupportLib)
endif(USE_SUPPORT_LIB)

if (CALIBRATE)
    if (DEFINED ENV{ROOTSYS})
        find_package(ROOT)
        if (ROOT_FOUND)
            add_subdirectory(calibrationWizards)
        endif()
    endif()
endif(CALIBRATE)

install(FILES ${ZMQ_STATIC_ARCHIVE}
DESTINATION lib) 
