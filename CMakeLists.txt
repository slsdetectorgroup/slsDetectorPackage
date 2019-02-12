cmake_minimum_required(VERSION 2.8)
# cmake_minimum_required(VERSION 3.5)
project(slsDetectorPackage)

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
option (SLS_USE_TESTS "TESTS" ON)

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

# set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=thread")
# set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=thread")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")



find_package(Qt4)
find_package(Qwt 6)
find_package(CBF)
find_package(Doxygen)

if (USE_TESTS)
    enable_testing()
endif(USE_TESTS)

# Support library containing functionallity common to 
# detector and receiver
add_subdirectory(slsSupportLib)

if (SLS_USE_HDF5)
	find_package(HDF5 1.10 COMPONENTS CXX)
endif (SLS_USE_HDF5)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_INSTALL_RPATH "$ORIGIN")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

#zmq
add_library(zmq STATIC IMPORTED GLOBAL)
set(ZMQ_STATIC_ARCHIVE ${CMAKE_CURRENT_SOURCE_DIR}/slsSupportLib/include/libzmq.a)
set_target_properties(zmq PROPERTIES
    IMPORTED_LOCATION ${ZMQ_STATIC_ARCHIVE}
)

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

add_subdirectory(integrationTests)

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
