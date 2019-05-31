add_executable(jungfrauDetectorServer
    slsDetectorFunctionList.c
    slsDetectorServer.c
    slsDetectorServer_funcs.c
    communication_funcs.c
)

target_include_directories(jungfrauDetectorServer
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(jungfrauDetectorServer
    PUBLIC JUNGFRAUD VIRTUAL STOP_SERVER
)

target_link_libraries(jungfrauDetectorServer
    PUBLIC pthread rt
)

set_target_properties(jungfrauDetectorServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS jungfrauDetectorServer
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)
