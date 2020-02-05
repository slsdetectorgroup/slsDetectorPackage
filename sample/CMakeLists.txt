add_executable(a api.cpp)
target_link_libraries(a 
    slsDetectorShared
    slsSupportLib
    pthread
    rt
)  

set_target_properties(a PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)


add_executable(result useResult.cpp)
target_link_libraries(result 
    slsDetectorShared
)  

set_target_properties(result PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_executable(udp udp.cpp)
target_link_libraries(udp 
    slsDetectorShared
    slsSupportLib
    pthread
    rt
    fmt
)  

set_target_properties(udp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)


