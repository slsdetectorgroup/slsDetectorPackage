

#Moench ZMQ
add_executable(moenchZmqProcess moenchZmqProcess.cpp ../tiffIO.cpp)
add_compile_definitions(moenchZmqProcess NEWZMQ INTERP)

#Moench04 ZMQ
add_executable(moench04ZmqProcess moenchZmqProcess.cpp ../tiffIO.cpp)
add_compile_definitions(moenchZmqProcess NEWZMQ INTERP MOENCH04)


#Both executables should have the same includes and output dirs
set(MOENCH_EXECUTABLES "moenchZmqProcess" "moench04ZmqProcess")

foreach(exe ${MOENCH_EXECUTABLES})
    #TODO! At a later stage clean up include dirs and have a proper lib
    target_include_directories(${exe} PRIVATE 
        ../ 
        ../dataStructures 
        ../interpolations
        ../../slsReceiverSoftware/include/
    )

    target_link_libraries(${exe} 
        PUBLIC
        slsSupportShared 
        ${ZeroMQ_LIBRARIES} 
        pthread 
        tiff

        PRIVATE
        slsProjectWarnings
    )


    set_target_properties(${exe}  PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endforeach(exe ${MOENCH_EXECUTABLES})
