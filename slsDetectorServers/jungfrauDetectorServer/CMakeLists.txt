# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
add_executable(jungfrauDetectorServer_virtual
    slsDetectorFunctionList.c
    ../slsDetectorServer/src/slsDetectorServer.c
    ../slsDetectorServer/src/slsDetectorServer_funcs.c
    ../slsDetectorServer/src/communication_funcs.c
    ../slsDetectorServer/src/blackfin.c
    ../slsDetectorServer/src/common.c
    ../slsDetectorServer/src/commonServerFunctions.c
    ../slsDetectorServer/src/AD9257.c
    ../slsDetectorServer/src/ALTERA_PLL.c
    ../slsDetectorServer/src/LTC2620.c
    ../slsDetectorServer/src/MAX1932.c
    ../slsDetectorServer/src/programViaBlackfin.c
    ../slsDetectorServer/src/communication_funcs_UDP.c    
    ../slsDetectorServer/src/sharedMemory.c
    ../../slsSupportLib/src/md5.c
)

target_include_directories(jungfrauDetectorServer_virtual
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    ../slsDetectorServer/include
    ../../slsSupportLib/include
)

target_compile_definitions(jungfrauDetectorServer_virtual
    PUBLIC JUNGFRAUD VIRTUAL STOP_SERVER #TEST_MOD_GEOMETRY
)

target_link_libraries(jungfrauDetectorServer_virtual
    PUBLIC pthread rt slsProjectCSettings
)

set_target_properties(jungfrauDetectorServer_virtual PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS jungfrauDetectorServer_virtual
    EXPORT "${TARGETS_EXPORT_NAME}"
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

configure_file(config_jungfrau.txt ${CMAKE_BINARY_DIR}/bin/config_jungfrau.txt COPYONLY)
configure_file(detid_jungfrau.txt ${CMAKE_BINARY_DIR}/bin/detid_jungfrau.txt COPYONLY)
