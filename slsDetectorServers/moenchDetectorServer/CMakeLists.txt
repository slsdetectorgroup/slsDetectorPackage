add_executable(moenchDetectorServer
    slsDetectorFunctionList.c
    ../slsDetectorServer/slsDetectorServer.c
    ../slsDetectorServer/slsDetectorServer_funcs.c
    ../slsDetectorServer/communication_funcs.c
)

include_directories(
    ../slsDetectorServer/
    ../../slsSupportLib/include
)

target_include_directories(moenchDetectorServer
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(moenchDetectorServer
    PUBLIC MOENCHD VIRTUAL STOP_SERVER
)

target_link_libraries(moenchDetectorServer
    PUBLIC pthread rt
)

set_target_properties(moenchDetectorServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS moenchDetectorServer
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)
