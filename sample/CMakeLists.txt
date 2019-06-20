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