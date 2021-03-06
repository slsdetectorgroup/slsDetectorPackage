add_executable(moenchDetectorServer_virtual
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
    ../slsDetectorServer/src/LTC2620.c
    ../slsDetectorServer/src/MAX1932.c
    ../slsDetectorServer/src/programFpgaBlackfin.c   
    ../slsDetectorServer/src/readDefaultPattern.c         
    ../slsDetectorServer/src/sharedMemory.c
)

include_directories(
    ../slsDetectorServer/include
    ../../slsSupportLib/include
    ../../slsDetectorSoftware/include/sls/
)

target_include_directories(moenchDetectorServer_virtual
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(moenchDetectorServer_virtual
    PUBLIC MOENCHD VIRTUAL STOP_SERVER
)

target_link_libraries(moenchDetectorServer_virtual
    PUBLIC pthread rt slsProjectCSettings
    m
)

set_target_properties(moenchDetectorServer_virtual PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS moenchDetectorServer_virtual
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)

configure_file(DefaultPattern_moench.txt ${CMAKE_BINARY_DIR}/bin/DefaultPattern_moench.txt COPYONLY)