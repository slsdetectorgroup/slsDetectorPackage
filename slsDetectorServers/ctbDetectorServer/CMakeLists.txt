# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
add_executable(ctbDetectorServer_virtual
    slsDetectorFunctionList.c
    ../slsDetectorServer/src/slsDetectorServer.c
    ../slsDetectorServer/src/slsDetectorServer_funcs.c
    ../slsDetectorServer/src/communication_funcs.c
    ../slsDetectorServer/src/blackfin.c
    ../slsDetectorServer/src/common.c
    ../slsDetectorServer/src/commonServerFunctions.c
    ../slsDetectorServer/src/communication_funcs_UDP.c
    ../slsDetectorServer/src/UDPPacketHeaderGenerator.c
    ../slsDetectorServer/src/AD9257.c
    ../slsDetectorServer/src/ALTERA_PLL.c
    ../slsDetectorServer/src/I2C.c
    ../slsDetectorServer/src/INA226.c
    ../slsDetectorServer/src/LTC2620.c
    ../slsDetectorServer/src/MAX1932.c
    ../slsDetectorServer/src/programViaBlackfin.c
    ../slsDetectorServer/src/sharedMemory.c
    ../slsDetectorServer/src/loadPattern.c
    ../../slsSupportLib/src/md5.c
)

include_directories(
    ../slsDetectorServer/include
    ../../slsSupportLib/include
    ../../slsDetectorSoftware/include/sls/
)

target_include_directories(ctbDetectorServer_virtual
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(ctbDetectorServer_virtual
    PUBLIC CHIPTESTBOARDD VIRTUAL STOP_SERVER
)

target_link_libraries(ctbDetectorServer_virtual
    PUBLIC pthread rt m slsProjectCSettings
)

set_target_properties(ctbDetectorServer_virtual PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS ctbDetectorServer_virtual
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)
