# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
add_executable(gotthard2DetectorServer_virtual
    slsDetectorFunctionList.c
    ../slsDetectorServer/src/slsDetectorServer.c
    ../slsDetectorServer/src/slsDetectorServer_funcs.c
    ../slsDetectorServer/src/communication_funcs.c
    ../slsDetectorServer/src/nios.c    
    ../slsDetectorServer/src/communication_funcs_UDP.c  
    ../slsDetectorServer/src/DAC6571.c  
    ../slsDetectorServer/src/common.c 
    ../slsDetectorServer/src/LTC2620_Driver.c
    ../slsDetectorServer/src/ALTERA_PLL_CYCLONE10.c    
    ../slsDetectorServer/src/ASIC_Driver.c
    ../slsDetectorServer/src/programViaNios.c
    ../slsDetectorServer/src/sharedMemory.c
    ../../slsSupportLib/src/md5.c
)

include_directories(
    ../slsDetectorServer/include
    ../../slsSupportLib/include
)

target_include_directories(gotthard2DetectorServer_virtual
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(gotthard2DetectorServer_virtual
    PUBLIC GOTTHARD2D VIRTUAL STOP_SERVER #DEBUG1 
)

target_link_libraries(gotthard2DetectorServer_virtual
    PUBLIC pthread rt slsProjectCSettings
)

set_target_properties(gotthard2DetectorServer_virtual PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS gotthard2DetectorServer_virtual
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)

configure_file(config_gotthard2.txt ${CMAKE_BINARY_DIR}/bin/config_gotthard2.txt COPYONLY)
configure_file(detid_gotthard2.txt ${CMAKE_BINARY_DIR}/bin/detid_gotthard2.txt COPYONLY)
