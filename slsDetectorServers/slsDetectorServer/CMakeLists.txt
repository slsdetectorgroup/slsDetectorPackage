add_library(slsDetectorServer
    slsDetectorServer_funcs.c
    slsDetectorServer.c
)


target_compile_definitions(slsDetectorServer
    PUBLIC JUNGFRAUD VIRTUAL STOP_SERVER
)


target_include_directories(slsDetectorServer
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)


target_link_libraries(slsDetectorServer
    PUBLIC slsSupportLib
)
