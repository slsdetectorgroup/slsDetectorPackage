# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
add_executable(gotthardDetectorServer_virtual
    slsDetectorFunctionList.c
    ../slsDetectorServer/src/slsDetectorServer.c
    ../slsDetectorServer/src/slsDetectorServer_funcs.c
    ../slsDetectorServer/src/communication_funcs.c
    ../slsDetectorServer/src/blackfin.c
    ../slsDetectorServer/src/AD9252.c
    ../slsDetectorServer/src/AD9257.c  
    ../slsDetectorServer/src/LTC2620.c         
    ../slsDetectorServer/src/common.c
    ../slsDetectorServer/src/commonServerFunctions.c    
    ../slsDetectorServer/src/programViaBlackfin.c
    ../slsDetectorServer/src/communication_funcs_UDP.c    
    ../slsDetectorServer/src/sharedMemory.c
    ../../slsSupportLib/src/md5.c
)

include_directories(
    ../slsDetectorServer/include
    ../../slsSupportLib/include
)

target_include_directories(gotthardDetectorServer_virtual
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(gotthardDetectorServer_virtual
    PUBLIC GOTTHARDD VIRTUAL STOP_SERVER
)

target_link_libraries(gotthardDetectorServer_virtual
    PUBLIC pthread rt slsProjectCSettings
) 

set_target_properties(gotthardDetectorServer_virtual PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS gotthardDetectorServer_virtual
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)

configure_file(config_gotthard.txt ${CMAKE_BINARY_DIR}/bin/config_gotthard.txt COPYONLY)