# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
add_executable(xilinx_ctbDetectorServer_virtual
    slsDetectorFunctionList.c
    ../slsDetectorServer/src/slsDetectorServer.c
    ../slsDetectorServer/src/slsDetectorServer_funcs.c
    ../slsDetectorServer/src/communication_funcs.c
    ../slsDetectorServer/src/arm64.c
    ../slsDetectorServer/src/common.c
    ../slsDetectorServer/src/sharedMemory.c
    ../slsDetectorServer/src/loadPattern.c
    ../slsDetectorServer/src/programViaArm.c
    ../slsDetectorServer/src/communication_funcs_UDP.c    
    ../../slsSupportLib/src/md5.c
    ../slsDetectorServer/src/LTC2620_Driver.c
)

include_directories(
    ../slsDetectorServer/include
    ../../slsSupportLib/include
    ../../slsDetectorSoftware/include/sls/
)

target_include_directories(xilinx_ctbDetectorServer_virtual
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(xilinx_ctbDetectorServer_virtual
    PUBLIC XILINX_CHIPTESTBOARDD ARMPROCESSOR VIRTUAL STOP_SERVER 
)

target_link_libraries(xilinx_ctbDetectorServer_virtual
    PUBLIC pthread rt m slsProjectCSettings
)

set_target_properties(xilinx_ctbDetectorServer_virtual PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS xilinx_ctbDetectorServer_virtual
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)
