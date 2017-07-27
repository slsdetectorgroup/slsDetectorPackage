cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(HDF5_ROOT_DIR /opt/hdf5v1.10.0)
find_package(Qt4)
find_package(Qwt 6)
find_package(CBF)
find_package(Doxygen)
find_package(HDF5 1.10)

set (REST OFF)
set (CALIBRATE OFF)


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



		
