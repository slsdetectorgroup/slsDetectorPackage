add_executable(gotthardDetectorServer
    slsDetectorFunctionList.c
    ../slsDetectorServer/slsDetectorServer.c
    ../slsDetectorServer/slsDetectorServer_funcs.c
    ../slsDetectorServer/communication_funcs.c
)

include_directories(
    ../slsDetectorServer/
    ../../slsSupportLib/include
)

target_include_directories(gotthardDetectorServer
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(gotthardDetectorServer
    PUBLIC GOTTHARDD VIRTUAL STOP_SERVER
)

target_link_libraries(gotthardDetectorServer
    PUBLIC pthread rt
)

set_target_properties(gotthardDetectorServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS gotthardDetectorServer
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)
