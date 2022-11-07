# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package



set(MOENCH_EXECUTABLES)

#Moench ZMQ
add_executable(moench03ZmqProcess moenchZmqProcess.cpp)
target_compile_definitions(moench03ZmqProcess PRIVATE NEWZMQ INTERP)
list(APPEND MOENCH_EXECUTABLES moench03ZmqProcess)

#Moench HighZ ZMQ
add_executable(moenchHighZZmqProcess moenchZmqProcess.cpp)
target_compile_definitions(moenchHighZZmqProcess PRIVATE NEWZMQ INTERP HIGHZ)
list(APPEND MOENCH_EXECUTABLES moenchHighZZmqProcess)

#Moench04 ZMQ
add_executable(moench04ZmqProcess moenchZmqProcess.cpp)
target_compile_definitions(moench04ZmqProcess PRIVATE NEWZMQ INTERP MOENCH04)
list(APPEND MOENCH_EXECUTABLES moench04ZmqProcess)


#OFFLINE Processing? 
add_executable(moench03RawDataProcess moenchRawDataProcess.cpp)
target_compile_definitions(moench03RawDataProcess PRIVATE)
list(APPEND MOENCH_EXECUTABLES moench03RawDataProcess)

add_executable(moenchHighZRawDataProcess moenchRawDataProcess.cpp)
target_compile_definitions(moenchHighZRawDataProcess PRIVATE  HIGHZ)
list(APPEND MOENCH_EXECUTABLES moenchHighZRawDataProcess)

add_executable(moench04RawDataProcess moenchRawDataProcess.cpp)
target_compile_definitions(moench04RawDataProcess PRIVATE  MOENCH04)
list(APPEND MOENCH_EXECUTABLES moench04RawDataProcess)

add_executable(moench04RawDataProcessDGS moenchRawDataProcess.cpp)
target_compile_definitions(moench04RawDataProcessDGS PRIVATE  MOENCH04_DGS MOENCH04)
list(APPEND MOENCH_EXECUTABLES moench04RawDataProcessDGS)

#interpolation stuff

add_executable(moench03MakeEta moench03Interpolation.cpp)
target_compile_definitions(moench03MakeEta PRIVATE FF)
list(APPEND MOENCH_EXECUTABLES moench03MakeEta)

add_executable(moench03Interpolation moench03Interpolation.cpp)
#no compile defs
list(APPEND MOENCH_EXECUTABLES moench03Interpolation)

add_executable(moench03NoInterpolation moench03NoInterpolation.cpp)
#no compile defs
list(APPEND MOENCH_EXECUTABLES moench03NoInterpolation)


foreach(exe ${MOENCH_EXECUTABLES})
    #TODO! At a later stage clean up include dirs and have a proper lib
    target_include_directories(${exe} PRIVATE 
        ../ 
        ../dataStructures 
        ../interpolations
        ../../slsReceiverSoftware/include/
	    ../../slsSupportLib/include/
        ${SLS_INTERNAL_RAPIDJSON_DIR}
    )

    target_link_libraries(${exe} 
        PUBLIC
        slsSupportStatic
        ${ZeroMQ_LIBRARIES} 
        pthread 
        tiffio

        PRIVATE
        slsProjectWarnings
        slsProjectOptions
    )


    set_target_properties(${exe}  PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    if((CMAKE_BUILD_TYPE STREQUAL "Release") AND SLS_LTO_AVAILABLE)
        set_property(TARGET ${exe} PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
    endif()

    
endforeach(exe ${MOENCH_EXECUTABLES})

install(TARGETS ${MOENCH_EXECUTABLES} DESTINATION bin)