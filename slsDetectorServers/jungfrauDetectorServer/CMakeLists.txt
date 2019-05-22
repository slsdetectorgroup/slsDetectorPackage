add_library(jungfrauDetectorServer
    slsDetectorFunctionList.c
    slsDetectorServer.c
)

target_include_directories(jungfrauDetectorServer
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(jungfrauDetectorServer
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS jungfrauDetectorServer
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)
