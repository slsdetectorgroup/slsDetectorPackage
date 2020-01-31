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
    ../slsDetectorServer/src/programFpgaBlackfin.c
    ../slsDetectorServer/src/communication_funcs_UDP.c    
)

include_directories(
    ../slsDetectorServer/include
    ../../slsSupportLib/include
)

target_include_directories(jungfrauDetectorServer_virtual
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(jungfrauDetectorServer_virtual
    PUBLIC JUNGFRAUD VIRTUAL STOP_SERVER
)

target_link_libraries(jungfrauDetectorServer_virtual
    PUBLIC pthread rt
)

set_target_properties(jungfrauDetectorServer_virtual PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS jungfrauDetectorServer_virtual
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)
