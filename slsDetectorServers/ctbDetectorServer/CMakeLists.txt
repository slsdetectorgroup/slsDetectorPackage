add_executable(ctbDetectorServer
    slsDetectorFunctionList.c
    ../slsDetectorServer/slsDetectorServer.c
    ../slsDetectorServer/slsDetectorServer_funcs.c
    ../slsDetectorServer/communication_funcs.c
)

include_directories(
    ../slsDetectorServer/
    ../../slsSupportLib/include
)

target_include_directories(ctbDetectorServer
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(ctbDetectorServer
    PUBLIC CHIPTESTBOARDD VIRTUAL STOP_SERVER
)

target_link_libraries(ctbDetectorServer
    PUBLIC pthread rt
)

set_target_properties(ctbDetectorServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS ctbDetectorServer
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)
