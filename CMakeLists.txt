cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set (CALIBRATE OFF)

# Check if project is being used directly or via add_subdirectory
set(SLS_MASTER_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
	set(SLS_MASTER_PROJECT ON)
endif()

option (SLS_USE_HDF5 "HDF5 File format" OFF)
option (SLS_USE_TEXTCLIENT "Text Client" OFF)
option (SLS_USE_RECEIVER "Receiver" OFF)
option (SLS_USE_GUI "GUI" OFF)


#Flags to always be used
# -Wno-unused-parameter should eventually be removes
# -Wno-overloaded-virtual TODO! remove warnings in refactor
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-overloaded-virtual -Wno-unused-parameter -std=c++98")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # clang does not support -Wno-misleading-indentation 
    # add any special clang flags
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6.0)
        set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-misleading-indentation")
    endif ()
endif()


find_package(Qt4)
find_package(Qwt 6)
find_package(CBF)
find_package(Doxygen)
# find_package(PNG REQUIRED)

if (SLS_USE_HDF5)
	find_package(HDF5 1.10 COMPONENTS CXX)
endif (SLS_USE_HDF5)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_INSTALL_RPATH "$ORIGIN")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)


if (SLS_USE_TEXTCLIENT)
       add_subdirectory(slsDetectorSoftware)
endif (SLS_USE_TEXTCLIENT)

if (SLS_USE_RECEIVER)
       add_subdirectory(slsReceiverSoftware)
       add_subdirectory(manual/manual-api)    
endif (SLS_USE_RECEIVER)
               
               
if (SLS_USE_GUI)   
       if (QT4_FOUND AND QWT_FOUND)
       add_subdirectory(slsDetectorGui)
       endif()
endif (SLS_USE_GUI)

if (SLS_MASTER_PROJECT)
	# Set targets export name (otherwise set by upstream project)
	set(TARGETS_EXPORT_NAME "slsdetector-targets")
endif (SLS_MASTER_PROJECT)

if (CALIBRATE)
    if (DEFINED ENV{ROOTSYS})
        find_package(ROOT)
        if (ROOT_FOUND)
            add_subdirectory(calibrationWizards)
        endif()
    endif()
endif(CALIBRATE)
