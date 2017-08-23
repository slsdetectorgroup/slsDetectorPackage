cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set (REST OFF)
set (CALIBRATE OFF)
option (USE_HDF5 "HDF5 File format" OFF)


find_package(Qt4)
find_package(Qwt 6)
find_package(CBF)
find_package(Doxygen)
if (USE_HDF5)
	find_package(HDF5 1.10 COMPONENTS CXX)
endif (USE_HDF5)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

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
