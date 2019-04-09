add_subdirectory(pybind11)
pybind11_add_module(_sls_detector src/main.cpp)



target_link_libraries(_sls_detector PUBLIC 
                        slsProjectOptions
                        slsProjectWarnings
                        slsDetectorShared
                        slsReceiverShared
                        slsSupportLib )  



set_target_properties(_sls_detector PROPERTIES
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
    errors.py
    experimental.py
    jungfrau_ctb.py
    jungfrau.py
    registers.py
    utils.py

)

foreach(FILE ${PYTHON_FILES})
    configure_file( sls_detector/${FILE}  
    ${CMAKE_BINARY_DIR}/bin/sls_detector/${FILE}  )

endforeach(FILE ${PYTHON_FILES})


configure_file( scripts/basic.py  
    ${CMAKE_BINARY_DIR}/basic.py 
)