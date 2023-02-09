# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
set(src 
    slsDetectorFunctionList.c
    ../slsDetectorServer/src/slsDetectorServer.c
    ../slsDetectorServer/src/slsDetectorServer_funcs.c
    ../slsDetectorServer/src/communication_funcs.c
    ../slsDetectorServer/src/communication_funcs_UDP.c
    ../slsDetectorServer/src/common.c
    ../slsDetectorServer/src/sharedMemory.c
    ../../slsSupportLib/src/md5.c
)

include_directories(
    ../slsDetectorServer/include
    ../../slsSupportLib/include
)

add_executable(eigerDetectorServer_virtual 
    ${src}
)

target_include_directories(eigerDetectorServer_virtual
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(eigerDetectorServer_virtual
    PUBLIC EIGERD PCCOMPILE STOP_SERVER #TEST_MOD_GEOMETRY
    PUBLIC VIRTUAL #VIRTUAL_9M
)

target_link_libraries(eigerDetectorServer_virtual
    PUBLIC pthread rt slsProjectCSettings
)

set_target_properties(eigerDetectorServer_virtual PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS eigerDetectorServer_virtual 
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)


configure_file(config_eiger.txt ${CMAKE_BINARY_DIR}/bin/config_eiger.txt COPYONLY)
configure_file(detid_eiger.txt ${CMAKE_BINARY_DIR}/bin/detid_eiger.txt COPYONLY)
