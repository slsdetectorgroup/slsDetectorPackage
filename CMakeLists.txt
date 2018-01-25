cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set (CALIBRATE OFF)

option (USE_HDF5 "HDF5 File format" OFF)
option (USE_TEXTCLIENT "Text Client" OFF)
option (USE_RECEIVER "Receiver" OFF)
option (USE_GUI "GUI" OFF)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wno-misleading-indentation")

find_package(Qt4)
find_package(Qwt 6)
find_package(CBF)
find_package(Doxygen)
find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})

if (USE_HDF5)
	find_package(HDF5 1.10 COMPONENTS CXX)
endif (USE_HDF5)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_INSTALL_RPATH "$ORIGIN")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)


if (USE_TEXTCLIENT)
       add_subdirectory(slsDetectorSoftware)
endif (USE_TEXTCLIENT)

if (USE_RECEIVER)
       add_subdirectory(slsReceiverSoftware)
endif (USE_RECEIVER)
               
if (USE_GUI)   
       if (QT4_FOUND AND QWT_FOUND)
       add_subdirectory(slsDetectorGui)
       endif()
endif (USE_GUI)


if (CALIBRATE)
    if (DEFINED ENV{ROOTSYS})
        find_package(ROOT)
        if (ROOT_FOUND)
            add_subdirectory(calibrationWizards)
        endif()
    endif()
endif(CALIBRATE)
