set(src 
    slsDetectorFunctionList.c
    slsDetectorServer.c
    slsDetectorServer_funcs.c
    communication_funcs.c
)

add_executable(eigerDetectorServerMaster 
    ${src}
)

target_include_directories(eigerDetectorServerMaster
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(eigerDetectorServerMaster
    PUBLIC EIGERD PCCOMPILE STOP_SERVER
    PUBLIC VIRTUAL DVIRTUAL_9M
    PUBLIC VIRTUAL_MASTER
)

target_link_libraries(eigerDetectorServerMaster
    PUBLIC pthread rt
)

install(TARGETS eigerDetectorServerMaster
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_executable(eigerDetectorServerSlave 
    ${src}
)

target_include_directories(eigerDetectorServerSlave
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(eigerDetectorServerSlave
    PUBLIC EIGERD PCCOMPILE STOP_SERVER
    PUBLIC VIRTUAL DVIRTUAL_9M
)

target_link_libraries(eigerDetectorServerSlave
    PUBLIC pthread rt
)

install(TARGETS eigerDetectorServerSlave
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)
