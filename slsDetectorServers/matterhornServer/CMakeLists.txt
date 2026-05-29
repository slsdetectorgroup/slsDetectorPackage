

set(MATTERHORN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MatterhornApp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SPICommunication.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HelperFunctions.cpp
)


if(SLS_USE_SIMULATOR)
    list(APPEND MATTERHORN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/VirtualMatterhornServer.cpp)

    set(MATTERHORN_SERVER_HEADER "VirtualMatterhornServer.h")

    add_executable(matterhornDetectorServer_virtual ${MATTERHORN_SOURCES})

    target_compile_definitions(matterhornDetectorServer_virtual PRIVATE
        MATTERHORN_SERVER_HEADER=<VirtualMatterhornServer.h>
        MATTERHORN_SERVER_CLASS=VirtualMatterhornServer
    )

    target_include_directories(matterhornDetectorServer_virtual
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../slsSupportLib/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../slsDetectorServer_cpp/include
        ${CMAKE_SOURCE_DIR}/slsDetectorSoftware/src) # because of SharedMemory TODO: should be in slsSupportLib?

    target_link_libraries(matterhornDetectorServer_virtual
        PUBLIC 
        slsSupportStatic
        slsServerStatic)

    set_target_properties(matterhornDetectorServer_virtual PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )

    install(TARGETS matterhornDetectorServer_virtual
        RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif() # maybe better to have a else if build with simulators on one always uses the virtual server in MatterhornApp 

list(APPEND MATTERHORN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/MatterhornServer.cpp)

#mmh should slsSupportLib be considered as well when updating version? 
if(CMAKE_TOOLCHAIN_FILE AND NOT CMAKE_TOOLCHAIN_FILE STREQUAL "") # only update version if cross compiling binaries 
    find_package(Python3 REQUIRED COMPONENTS Interpreter REQUIRED)
    add_custom_target(update_server_version ALL
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/etc/updateAPIVersion.py APIMATTERHORN ${CMAKE_SOURCE_DIR}/slsDetectorServers/matterhornServer ${CMAKE_SOURCE_DIR}/slsDetectorServers/slsDetectorServer_cpp ${CMAKE_SOURCE_DIR}/slsSupportLib  
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Updating APIMATTERHORN version"
        VERBATIM)
endif() 

add_executable(matterhornDetectorServer ${MATTERHORN_SOURCES})

if(CMAKE_TOOLCHAIN_FILE AND NOT CMAKE_TOOLCHAIN_FILE STREQUAL "") # only update version if cross compiling binaries 
    add_dependencies(matterhornDetectorServer update_server_version)
endif()

target_compile_definitions(matterhornDetectorServer PRIVATE
    MATTERHORN_SERVER_HEADER=<MatterhornServer.h>
    MATTERHORN_SERVER_CLASS=MatterhornServer
)

target_include_directories(matterhornDetectorServer
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../slsSupportLib/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../slsDetectorServer_cpp/include)
    
target_link_libraries(matterhornDetectorServer
    PUBLIC
    slsSupportStatic
    slsServerStatic)

set_target_properties(matterhornDetectorServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS matterhornDetectorServer
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)


if(SLS_USE_TESTS)
    add_subdirectory(tests)
endif()


#target_compile_definitions(matterhornDetectorServer_virtual
#    PUBLIC VIRTUAL STOP_SERVER #what is this stop server should we really have a generic ServerAPP and pass compile options to create server e.g. MatterHorn? 
#)




