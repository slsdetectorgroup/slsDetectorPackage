# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package


# Initial draft for Makefile.rawdataprocess

set(JUNGFRAU_EXECUTABLES)

find_package(fmt REQUIRED)

# jungfrauRawDataProcess
add_executable(jungfrauRawDataProcess jungfrauRawDataProcess.cpp)
target_compile_definitions(jungfrauRawDataProcess PRIVATE MODULE)
list(APPEND JUNGFRAU_EXECUTABLES jungfrauRawDataProcess)

# jungfrauRawDataProcessStrx
add_executable(jungfrauRawDataProcessStrx jungfrauRawDataProcess_filetxt.cpp)
target_compile_definitions(jungfrauRawDataProcessStrx PRIVATE JFSTRX)
list(APPEND JUNGFRAU_EXECUTABLES jungfrauRawDataProcessStrx)


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




foreach(exe ${JUNGFRAU_EXECUTABLES})
    #TODO! At a later stage clean up include dirs and have a proper lib
    target_include_directories(${exe} PRIVATE 
        ../ 
        ../interpolations
        ../dataStructures 
        ../interpolations/etaVEL
        ../../slsSupportLib/include/
        ../../slsReceiverSoftware/include/
        ../tiffio/include
        ${fmt_INCLUDE_DIRS}
    )

    target_link_libraries(${exe} 
        PUBLIC
        slsSupportStatic
        pthread 
        tiffio
        fmt::fmt
          #-L/usr/lib64/
        #-lm  -lstdc++  -lrt

        PRIVATE
        slsProjectWarnings
        slsProjectOptions
    )


    set_target_properties(${exe}  PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    if((CMAKE_BUILD_TYPE STREQUAL "Release") AND SLS_LTO_AVAILABLE)
        set_property(TARGET ${exe} PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
    endif()

    
endforeach(exe ${JUNGFRAU_EXECUTABLES})

install(TARGETS ${JUNGFRAU_EXECUTABLES} DESTINATION bin)