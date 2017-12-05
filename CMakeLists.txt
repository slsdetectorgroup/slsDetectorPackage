cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set (CALIBRATE OFF)
option (USE_HDF5 "HDF5 File format" OFF)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall")

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

add_subdirectory(slsDetectorSoftware)
add_subdirectory(slsReceiverSoftware)
if (QT4_FOUND AND QWT_FOUND)
    add_subdirectory(slsDetectorGui)
endif()


if (CALIBRATE)
    if (DEFINED ENV{ROOTSYS})
        find_package(ROOT)
        if (ROOT_FOUND)
            add_subdirectory(calibrationWizards)
        endif()
    endif()
endif(CALIBRATE)
