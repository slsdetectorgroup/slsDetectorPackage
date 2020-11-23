
pybind11_add_module(_slsdet 
    src/main.cpp 
    src/enums.cpp
    src/detector.cpp
    src/network.cpp
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