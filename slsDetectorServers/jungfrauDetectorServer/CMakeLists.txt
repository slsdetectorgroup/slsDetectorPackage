add_library(jungfrauDetectorServer
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

install(TARGETS jungfrauDetectorServer
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)
