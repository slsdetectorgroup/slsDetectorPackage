set(src 
    slsDetectorFunctionList.c
    ../slsDetectorServer/src/slsDetectorServer.c
    ../slsDetectorServer/src/slsDetectorServer_funcs.c
    ../slsDetectorServer/src/communication_funcs.c
    ../slsDetectorServer/src/communication_funcs_UDP.c
    ../slsDetectorServer/src/common.c
    ../slsDetectorServer/src/communication_virtual.c
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
    PUBLIC VIRTUAL #VIRTUAL_9M
    PUBLIC VIRTUAL_MASTER
)

target_link_libraries(eigerDetectorServerMaster_virtual
    PUBLIC pthread rt slsProjectCSettings
)

set_target_properties(eigerDetectorServerMaster_virtual PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS eigerDetectorServerMaster_virtual 
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)





add_executable(eigerDetectorServerSlaveTop_virtual 
    ${src}
)

target_include_directories(eigerDetectorServerSlaveTop_virtual
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(eigerDetectorServerSlaveTop_virtual
    PUBLIC EIGERD PCCOMPILE STOP_SERVER
    PUBLIC VIRTUAL #VIRTUAL_9M
    PUBLIC VIRTUAL_TOP
)

target_link_libraries(eigerDetectorServerSlaveTop_virtual
    PUBLIC pthread rt slsProjectCSettings
)

set_target_properties(eigerDetectorServerSlaveTop_virtual PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS eigerDetectorServerSlaveTop_virtual
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)




add_executable(eigerDetectorServerSlaveBottom_virtual 
    ${src}
)

target_include_directories(eigerDetectorServerSlaveBottom_virtual
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(eigerDetectorServerSlaveBottom_virtual
    PUBLIC EIGERD PCCOMPILE STOP_SERVER
    PUBLIC VIRTUAL #VIRTUAL_9M
)

target_link_libraries(eigerDetectorServerSlaveBottom_virtual
    PUBLIC pthread rt slsProjectCSettings
)

set_target_properties(eigerDetectorServerSlaveBottom_virtual PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS eigerDetectorServerSlaveBottom_virtual
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)
