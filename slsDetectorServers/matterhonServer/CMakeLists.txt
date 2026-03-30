# TODO: should be different executable if not simulators on !! 
add_executable(matterhornDetectorServer_virtual
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MatterhornApp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VirtualMatterhornServer.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/src/StopServer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CommandLineOptions.cpp
)

target_include_directories(matterhornDetectorServer_virtual
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../slsSupportLib/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../slsDetectorServer/include 
)

target_compile_definitions(matterhornDetectorServer_virtual
    PUBLIC VIRTUAL STOP_SERVER #what is this stop server should we really have a generic ServerAPP and pass compile options to create server e.g. MatterHorn? 
)

target_link_libraries(matterhornDetectorServer_virtual
    PUBLIC 
        slsSupportStatic
        slsDetectorStatic
        slsServerStatic
)

set_target_properties(matterhornDetectorServer_virtual PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS matterhornDetectorServer_virtual
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)


