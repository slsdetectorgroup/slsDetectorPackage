# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    #if we are only building the python extension
    cmake_minimum_required(VERSION 3.12)    
    project(slsdet)
    find_package(slsDetectorPackage 6 REQUIRED)
endif()


find_package (Python 3.6 COMPONENTS Interpreter Development)
add_subdirectory(../libs/pybind11  ${CMAKE_BINARY_DIR}/bin/)

pybind11_add_module(_slsdet 
    src/main.cpp 
    src/enums.cpp
    src/detector.cpp
    src/network.cpp
    src/pattern.cpp
    src/scan.cpp
    src/current.cpp
)

target_link_libraries(_slsdet PUBLIC 
                      slsDetectorStatic
)  



set_target_properties(_slsdet PROPERTIES
                        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
                    )

#Copy Python code 
set( PYTHON_FILES
    __init__.py
    adcs.py
    dacs.py
    decorators.py
    detector_property.py
    detector.py
    eiger.py
    enums.py
    errors.py
    gotthard.py
    pattern.py
    gotthard2.py
    moench.py
    proxy.py
    ctb.py
    jungfrau.py
    mythen3.py
    registers.py
    temperature.py
    lookup.py
    utils.py

)

foreach(FILE ${PYTHON_FILES})
    configure_file( slsdet/${FILE}  
    ${CMAKE_BINARY_DIR}/bin/slsdet/${FILE}  )

endforeach(FILE ${PYTHON_FILES})


configure_file( scripts/basic.py  
    ${CMAKE_BINARY_DIR}/basic.py 
)
configure_file( scripts/test_virtual.py  
    ${CMAKE_BINARY_DIR}/test_virtual.py 
)