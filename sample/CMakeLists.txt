# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
add_executable(using_logger using_logger.cpp)
target_link_libraries(using_logger 
    slsSupportShared
    pthread
    rt
)  

set_target_properties(using_logger PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)


# add_executable(result useResult.cpp)
# target_link_libraries(result 
#     slsDetectorShared
# )  

# set_target_properties(result PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
# )

# add_executable(udp udp.cpp)
# target_link_libraries(udp 
#     slsDetectorShared
#     slsSupportShared
#     pthread
#     rt
#     fmt
# )  

# set_target_properties(udp PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
# )


