add_executable(gotthard2DetectorServer_virtual
    slsDetectorFunctionList.c
    ../slsDetectorServer/slsDetectorServer.c
    ../slsDetectorServer/slsDetectorServer_funcs.c
    ../slsDetectorServer/communication_funcs.c
)

include_directories(
    ../slsDetectorServer/
    ../../slsSupportLib/include
)

target_include_directories(gotthard2DetectorServer_virtual
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(gotthard2DetectorServer_virtual
    PUBLIC GOTTHARD2D VIRTUAL STOP_SERVER
)

target_link_libraries(gotthard2DetectorServer_virtual
    PUBLIC pthread rt
)

set_target_properties(gotthard2DetectorServer_virtual PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS gotthard2DetectorServer_virtual
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)
