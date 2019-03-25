set(SOURCES
    src/Listener.cpp
)

add_library(slsListenerShared SHARED
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(slsListenerShared PUBLIC
    slsSupportLib
)

target_include_directories(slsListenerShared PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

#What is included in slsReceiverLib?
set(PUBLICHEADERS
    include/Listener.h
)


set_target_properties(slsListenerShared PROPERTIES
    LIBRARY_OUTPUT_NAME slsListener
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    PUBLIC_HEADER "${PUBLICHEADERS}"
)

install(TARGETS slsListenerShared
        EXPORT "${TARGETS_EXPORT_NAME}"
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
