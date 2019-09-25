add_executable(mythen3DetectorServer_virtual
    slsDetectorFunctionList.c
    ../slsDetectorServer/src/slsDetectorServer.c
    ../slsDetectorServer/src/slsDetectorServer_funcs.c
    ../slsDetectorServer/src/communication_funcs.c
    # ../slsDetectorServer/src/blackfin.c    
    ../slsDetectorServer/src/communication_funcs_UDP.c  
    ../slsDetectorServer/src/DAC6571.c      
    ../slsDetectorServer/src/common.c 
    ../slsDetectorServer/src/nios.c  
)

include_directories(
    ../slsDetectorServer/include
    ../../slsSupportLib/include
)

target_include_directories(mythen3DetectorServer_virtual
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(mythen3DetectorServer_virtual
    PUBLIC MYTHEN3D VIRTUAL STOP_SERVER
)

target_link_libraries(mythen3DetectorServer_virtual
    PUBLIC pthread rt
)

set_target_properties(mythen3DetectorServer_virtual PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS mythen3DetectorServer_virtual
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)
