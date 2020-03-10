set(src 
    slsDetectorFunctionList.c
    ../slsDetectorServer/src/slsDetectorServer.c
    ../slsDetectorServer/src/slsDetectorServer_funcs.c
    ../slsDetectorServer/src/communication_funcs.c
    ../slsDetectorServer/src/communication_funcs_UDP.c
    ../slsDetectorServer/src/common.c
)

include_directories(
    ../slsDetectorServer/include
    ../../slsSupportLib/include
)

add_executable(eigerDetectorServerMaster_virtual 
    ${src}
)

target_include_directories(eigerDetectorServerMaster_virtual
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(eigerDetectorServerMaster_virtual
    PUBLIC EIGERD PCCOMPILE STOP_SERVER
    PUBLIC VIRTUAL DVIRTUAL_9M
    PUBLIC VIRTUAL_MASTER
)

target_link_libraries(eigerDetectorServerMaster_virtual
    PUBLIC pthread rt slsProjectCWarnings
)

set_target_properties(eigerDetectorServerMaster_virtual PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS eigerDetectorServerMaster_virtual 
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_executable(eigerDetectorServerSlave_virtual 
    ${src}
)

target_include_directories(eigerDetectorServerSlave_virtual
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(eigerDetectorServerSlave_virtual
    PUBLIC EIGERD PCCOMPILE STOP_SERVER
    PUBLIC VIRTUAL DVIRTUAL_9M
)

target_link_libraries(eigerDetectorServerSlave_virtual
    PUBLIC pthread rt
)

set_target_properties(eigerDetectorServerSlave_virtual PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS eigerDetectorServerSlave_virtual
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)
