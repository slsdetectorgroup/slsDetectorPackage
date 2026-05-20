

set(MATTERHORN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MatterhornApp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SPICommunication.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HelperFunctions.cpp
)

if(SLS_USE_SIMULATOR)
    list(APPEND MATTERHORN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/VirtualMatterhornServer.cpp)

    add_executable(matterhornDetectorServer_virtual ${MATTERHORN_SOURCES})

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
else()
    list(APPEND MATTERHORN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/MatterhornServer.cpp)

    add_executable(matterhornDetectorServer ${MATTERHORN_SOURCES})

    target_include_directories(matterhornDetectorServer
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../slsSupportLib/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../slsDetectorServer_cpp/include)
    
    target_link_libraries(matterhornDetectorServer
        PUBLIC
        slsSupportStatic
        #slsDetectorStatic
        slsServerStatic)

    set_target_properties(matterhornDetectorServer PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )

    install(TARGETS matterhornDetectorServer
        RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

endif()

if(SLS_USE_TESTS)
    add_subdirectory(tests)
endif()


#target_compile_definitions(matterhornDetectorServer_virtual
#    PUBLIC VIRTUAL STOP_SERVER #what is this stop server should we really have a generic ServerAPP and pass compile options to create server e.g. MatterHorn? 
#)




