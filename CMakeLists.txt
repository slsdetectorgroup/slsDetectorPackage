cmake_minimum_required(VERSION 2.8)

#project(slsDetectorPackage LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set (CALIBRATE OFF)

option (USE_HDF5 "HDF5 File format" OFF)
option (USE_TEXTCLIENT "Text Client" OFF)
option (USE_RECEIVER "Receiver" OFF)
option (USE_GUI "GUI" OFF)
option (USE_TESTS "TESTS" ON)

#Testing for minimum version for compilers
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # clang does not support -Wno-misleading-indentation 
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.2)
        message(FATAL_ERROR "Clang version must be at least 3.2!")
    endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
            message(FATAL_ERROR "GCC version must be at least 4.8!")
    endif()
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-misleading-indentation")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 ")

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=thread")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=thread")

find_package(Qt4)
find_package(Qwt 6)
find_package(CBF)
find_package(Doxygen)

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

if (USE_TESTS)
    add_subdirectory(tests)
endif(USE_TESTS)

install(FILES ${ZMQ_STATIC_ARCHIVE}
DESTINATION lib) 
