

set(MATTERHORN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MatterhornApp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/communication/SPICommunication.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/HelperFunctions.cpp
)

set(MATTERHORN_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/communication
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/src/defs
    ${CMAKE_CURRENT_SOURCE_DIR}/../../slsSupportLib/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../slsDetectorServer_cpp/include
)

if(SLS_USE_SIMULATOR)
    set(MATTERHORN_SERVER_HEADER "VirtualMatterhornServer.hpp")

    add_executable(matterhornDetectorServer_virtual ${MATTERHORN_SOURCES})

    target_compile_definitions(matterhornDetectorServer_virtual PRIVATE
        MATTERHORN_SERVER_HEADER=<VirtualMatterhornServer.hpp>
        MATTERHORN_SERVER_CLASS=VirtualMatterhornServer
    )

    target_include_directories(matterhornDetectorServer_virtual
        PRIVATE ${MATTERHORN_INCLUDE_DIRS})

    target_link_libraries(matterhornDetectorServer_virtual
        PUBLIC 
        slsSupportStatic
        slsServerStatic
        slsProjectOptions
        PRIVATE
        slsProjectWarnings
    )

    set_target_properties(matterhornDetectorServer_virtual PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )

    install(TARGETS matterhornDetectorServer_virtual
        RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif() # maybe better to have a else if build with simulators on one always uses the virtual server in MatterhornApp 

if(SLS_USE_MATTERHORN) 


    if(CMAKE_CROSSCOMPILING) # only update version if cross compiling binaries 
        find_package(Python3 REQUIRED COMPONENTS Interpreter REQUIRED)
        add_custom_target(update_server_version ALL
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/etc/updateAPIVersion.py APIMATTERHORN ${CMAKE_SOURCE_DIR}/slsDetectorServers/matterhornServer ${CMAKE_SOURCE_DIR}/slsDetectorServers/slsDetectorServer_cpp   
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Updating APIMATTERHORN version"
            VERBATIM)
    endif() 

    add_executable(matterhornDetectorServer ${MATTERHORN_SOURCES})

    if(CMAKE_CROSSCOMPILING) # only update version if cross compiling binaries 
        add_dependencies(matterhornDetectorServer update_server_version)
    endif()

    target_compile_definitions(matterhornDetectorServer PRIVATE
        MATTERHORN_SERVER_HEADER=<MatterhornServer.hpp>
        MATTERHORN_SERVER_CLASS=MatterhornServer
    )

    target_include_directories(matterhornDetectorServer
            PRIVATE ${MATTERHORN_INCLUDE_DIRS})
        
    target_link_libraries(matterhornDetectorServer
        PUBLIC
        slsSupportStatic
        slsServerStatic
        slsProjectOptions
        PRIVATE
        slsProjectWarnings
    )

    if(CMAKE_CROSSCOMPILING) # change output directory to stay consitent with c server binaries when cross compiling
        set(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
    else()
        set(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    endif()

    set_target_properties(matterhornDetectorServer PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${RUNTIME_OUTPUT_DIRECTORY}
    )

    install(TARGETS matterhornDetectorServer
        RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

if(SLS_USE_TESTS)
    add_subdirectory(tests)
endif()



