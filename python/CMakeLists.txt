# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package

pybind11_add_module(_slsdet 
    src/main.cpp 
    src/enums.cpp
    src/detector.cpp
    src/network.cpp
    src/pattern.cpp
    src/scan.cpp
    src/current.cpp
    src/duration.cpp
    src/DurationWrapper.cpp
    src/pedestal.cpp
)

target_link_libraries(_slsdet PUBLIC 
                      slsDetectorStatic
)  


set_target_properties(_slsdet PROPERTIES
                        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
                    )

#Copy Python code 
set( PYTHON_FILES
    slsdet/__init__.py
    slsdet/adcs.py
    slsdet/dacs.py
    slsdet/powers.py
    slsdet/decorators.py
    slsdet/detector_property.py
    slsdet/detector.py
    slsdet/defines.py
    slsdet/eiger.py
    slsdet/enums.py
    slsdet/errors.py
    slsdet/gaincaps.py
    slsdet/gotthard.py
    slsdet/pattern.py
    slsdet/gotthard2.py
    slsdet/moench.py
    slsdet/proxy.py
    slsdet/ctb.py
    slsdet/jungfrau.py
    slsdet/mythen3.py
    slsdet/registers.py
    slsdet/temperature.py
    slsdet/lookup.py
    slsdet/utils.py
)

foreach(FILE ${PYTHON_FILES})
    configure_file( ${FILE}  
    ${CMAKE_BINARY_DIR}/bin/${FILE}  )

endforeach(FILE ${PYTHON_FILES})

configure_file( scripts/basic.py  
    ${CMAKE_BINARY_DIR}/basic.py 
)
configure_file( scripts/test_virtual.py  
    ${CMAKE_BINARY_DIR}/test_virtual.py 
)

configure_file( ${CMAKE_CURRENT_LIST_DIR}/../VERSION  
    ${CMAKE_BINARY_DIR}/bin/slsdet/VERSION
)

if(SLS_INSTALL_PYTHONEXT)
    install(TARGETS _slsdet
        EXPORT "${TARGETS_EXPORT_NAME}"
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/python
    )

    install(FILES ${PYTHON_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/python/slsdet)
    install(FILES ../VERSION DESTINATION ${CMAKE_INSTALL_PREFIX}/python/slsdet)
endif()