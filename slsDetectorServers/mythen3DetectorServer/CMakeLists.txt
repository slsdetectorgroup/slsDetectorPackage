add_executable(mythen3DetectorServer
    slsDetectorFunctionList.c
    ../slsDetectorServer/slsDetectorServer.c
    ../slsDetectorServer/slsDetectorServer_funcs.c
    ../slsDetectorServer/communication_funcs.c
)

include_directories(
    ../slsDetectorServer/
    ../../slsSupportLib/include
)

target_include_directories(mythen3DetectorServer
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(mythen3DetectorServer
    PUBLIC MYTHEN3D VIRTUAL STOP_SERVER
)

target_link_libraries(mythen3DetectorServer
    PUBLIC pthread rt
)

set_target_properties(mythen3DetectorServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS mythen3DetectorServer
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)
