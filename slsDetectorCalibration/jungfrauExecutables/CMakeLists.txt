# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package


# Initial draft for Makefile.rawdataprocess

set(JUNGFRAU_EXECUTABLES)

find_package(fmt REQUIRED)
find_package(nlohmann_json 3.11.2 REQUIRED)

# HDF5 file writing 
if (SLS_USE_HDF5)
    find_package(HDF5 1.10 COMPONENTS CXX REQUIRED)
	    list (APPEND SOURCES 
	        HDF5File.cpp 
	    )
endif (SLS_USE_HDF5)



# jungfrauRawDataProcess
add_executable(jungfrauRawDataProcess jungfrauRawDataProcess.cpp)
target_compile_definitions(jungfrauRawDataProcess PRIVATE MODULE)
list(APPEND JUNGFRAU_EXECUTABLES jungfrauRawDataProcess)

# jungfrauRawDataProcessChipAldo
add_executable(jungfrauRawDataProcessChipAldo jungfrauRawDataProcess_filetxt.cpp)
target_compile_definitions(jungfrauRawDataProcessChipAldo PRIVATE CHIP ALDO)
list(APPEND JUNGFRAU_EXECUTABLES jungfrauRawDataProcessChipAldo)

# jungfrauRawDataProcessStrx
add_executable(jungfrauRawDataProcessStrx jungfrauRawDataProcess_filetxt.cpp)
target_compile_definitions(jungfrauRawDataProcessStrx PRIVATE JFSTRX)
list(APPEND JUNGFRAU_EXECUTABLES jungfrauRawDataProcessStrx)

# jungfrauRawDataProcessStrxQuad
add_executable(jungfrauRawDataProcessStrxQuad jungfrauRawDataProcess_filetxt.cpp)
target_compile_definitions(jungfrauRawDataProcessStrxQuad PRIVATE JFSTRXQ)
list(APPEND JUNGFRAU_EXECUTABLES jungfrauRawDataProcessStrxQuad)

# jungfrauRawDataProcessStrxQuadH5
# HDF5
if (SLS_USE_HDF5)
    if (HDF5_FOUND)
       add_executable(jungfrauRawDataProcessStrxQuadH5 jungfrauRawDataProcess_filetxtH5.cpp)
       #target_compile_definitions(jungfrauRawDataProcessStrxQuadH5 PRIVATE JFSTRXQH5)
       list(APPEND JUNGFRAU_EXECUTABLES jungfrauRawDataProcessStrxQuadH5)
       target_include_directories(jungfrauRawDataProcessStrxQuadH5 PRIVATE ${HDF5_INCLUDE_DIRS}  ${CMAKE_INSTALL_PREFIX}/include)
       target_link_libraries(jungfrauRawDataProcessStrxQuadH5 PRIVATE ${HDF5_LIBRARIES})
    endif ()
endif (SLS_USE_HDF5)

# jungfrauRawDataProcessStrxChip1
add_executable(jungfrauRawDataProcessStrxChip1 jungfrauRawDataProcess.cpp)
target_compile_definitions(jungfrauRawDataProcessStrxChip1 PRIVATE JFSTRXCHIP1)
list(APPEND JUNGFRAU_EXECUTABLES jungfrauRawDataProcessStrxChip1)

# jungfrauRawDataProcessStrxChip6
add_executable(jungfrauRawDataProcessStrxChip6 jungfrauRawDataProcess.cpp)
target_compile_definitions(jungfrauRawDataProcessStrxChip6 PRIVATE JFSTRXCHIP6)
list(APPEND JUNGFRAU_EXECUTABLES jungfrauRawDataProcessStrxChip6)

# jungfrauRawDataProcessStrxChip1Aldo
add_executable(jungfrauRawDataProcessStrxChip1Aldo jungfrauRawDataProcess.cpp)
target_compile_definitions(jungfrauRawDataProcessStrxChip1Aldo PRIVATE JFSTRXCHIP1 ALDO)
list(APPEND JUNGFRAU_EXECUTABLES jungfrauRawDataProcessStrxChip1Aldo)

# jungfrauRawDataProcessStrxChip6Aldo
add_executable(jungfrauRawDataProcessStrxChip6Aldo jungfrauRawDataProcess.cpp)
target_compile_definitions(jungfrauRawDataProcessStrxChip6Aldo PRIVATE JFSTRXCHIP6 ALDO)
list(APPEND JUNGFRAU_EXECUTABLES jungfrauRawDataProcessStrxChip6Aldo)

# jungfrauRawDataProcessStrxAldo
add_executable(jungfrauRawDataProcessStrxAldo jungfrauRawDataProcess.cpp)
target_compile_definitions(jungfrauRawDataProcessStrxAldo PRIVATE JFSTRX ALDO)
list(APPEND JUNGFRAU_EXECUTABLES jungfrauRawDataProcessStrxAldo)

# jungfrauRawDataProcessStrxOld
add_executable(jungfrauRawDataProcessStrxOld jungfrauRawDataProcess.cpp)
target_compile_definitions(jungfrauRawDataProcessStrxOld PRIVATE JFSTRXOLD)
list(APPEND JUNGFRAU_EXECUTABLES jungfrauRawDataProcessStrxOld)

# jungfrauRawDataProcessStrxOldAldo
add_executable(jungfrauRawDataProcessStrxOldAldo jungfrauRawDataProcess.cpp)
target_compile_definitions(jungfrauRawDataProcessStrxOldAldo PRIVATE JFSTRXOLD ALDO)
list(APPEND JUNGFRAU_EXECUTABLES jungfrauRawDataProcessStrxOldAldo)


# others to be added if needed (might already be there in Makefile.cluster_finder TO BE CHECKED)


if (SLS_USE_HDF5)
   if (HDF5_FOUND)
        target_include_directories(jungfrauRawDataProcessStrxQuadH5 PRIVATE 
								    ${HDF5_INCLUDE_DIRS}
								    ${CMAKE_INSTALL_PREFIX}/include
								    )
	target_link_libraries(jungfrauRawDataProcessStrxQuadH5 PRIVATE ${HDF5_LIBRARIES})

   endif ()
endif (SLS_USE_HDF5)


foreach(exe ${JUNGFRAU_EXECUTABLES})
    #TODO! At a later stage clean up include dirs and have a proper lib
    target_include_directories(${exe} PRIVATE 
        ../ 
        ../interpolations
        ../dataStructures 
        ../interpolations/etaVEL
        ../../slsSupportLib/include/sls/
        ../../slsReceiverSoftware/include/
        ../tiffio/include
        ${fmt_INCLUDE_DIRS}
    )
#    if (SLS_USE_HDF5)
#       if (HDF5_FOUND)
#       	  target_include_directories(${exe} PRIVATE ${HDF5_INCLUDE_DIRS}  ${CMAKE_INSTALL_PREFIX}/include)
#       endif ()
#    endif (SLS_USE_HDF5)

    target_link_libraries(${exe} 
        PUBLIC
        slsSupportStatic
        pthread 
        tiffio
        fmt::fmt
	nlohmann_json::nlohmann_json
          #-L/usr/lib64/
        #-lm  -lstdc++  -lrt

        PRIVATE
        slsProjectWarnings
        slsProjectOptions
    )
#    if (SLS_USE_HDF5)
#       if (HDF5_FOUND)
#	target_link_libraries(${exe} PRIVATE ${HDF5_LIBRARIES})
#       endif ()
#    endif (SLS_USE_HDF5)

    set_target_properties(${exe}  PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    if((CMAKE_BUILD_TYPE STREQUAL "Release") AND SLS_LTO_AVAILABLE)
        set_property(TARGET ${exe} PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
    endif()

    
endforeach(exe ${JUNGFRAU_EXECUTABLES})





install(TARGETS ${JUNGFRAU_EXECUTABLES} DESTINATION bin)
