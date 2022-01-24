# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package



set(MOENCH_EXECUTABLES)

#Moench ZMQ
add_executable(moenchZmqProcess moenchZmqProcess.cpp)
target_compile_definitions(moenchZmqProcess PRIVATE NEWZMQ INTERP)
list(APPEND MOENCH_EXECUTABLES moenchZmqProcess)

#Moench04 ZMQ
add_executable(moench04ZmqProcess moenchZmqProcess.cpp)
target_compile_definitions(moench04ZmqProcess PRIVATE NEWZMQ INTERP MOENCH04)
list(APPEND MOENCH_EXECUTABLES moench04ZmqProcess)


#OFFLINE Processing? 
add_executable(moenchClusterFinder moench03ClusterFinder.cpp)
target_compile_definitions(moenchClusterFinder PRIVATE SAVE_ALL  NEWRECEIVER)
list(APPEND MOENCH_EXECUTABLES moenchClusterFinder)

add_executable(moenchClusterFinderHighZ moench03ClusterFinder.cpp)
target_compile_definitions(moenchClusterFinderHighZ PRIVATE SAVE_ALL  NEWRECEIVER HIGHZ)
list(APPEND MOENCH_EXECUTABLES moenchClusterFinderHighZ)

add_executable(moenchMakeEta moench03Interpolation.cpp)
target_compile_definitions(moenchMakeEta PRIVATE FF)
list(APPEND MOENCH_EXECUTABLES moenchMakeEta)

add_executable(moenchInterpolation moench03Interpolation.cpp)
#no compile defs
list(APPEND MOENCH_EXECUTABLES moenchInterpolation)

add_executable(moenchNoInterpolation moench03NoInterpolation.cpp)
#no compile defs
list(APPEND MOENCH_EXECUTABLES moenchNoInterpolation)

add_executable(moenchPhotonCounter moenchPhotonCounter.cpp)
target_compile_definitions(moenchPhotonCounter PRIVATE NEWRECEIVER)
list(APPEND MOENCH_EXECUTABLES moenchPhotonCounter)

add_executable(moenchAnalog moenchPhotonCounter.cpp)
target_compile_definitions(moenchAnalog PRIVATE NEWRECEIVER ANALOG)
list(APPEND MOENCH_EXECUTABLES moenchAnalog)

add_executable(moenchPhotonCounterHighZ moenchPhotonCounter.cpp)
target_compile_definitions(moenchPhotonCounterHighZ PRIVATE NEWRECEIVER HIGHZ)
list(APPEND MOENCH_EXECUTABLES moenchPhotonCounterHighZ)

add_executable(moenchAnalogHighZ moenchPhotonCounter.cpp)
target_compile_definitions(moenchAnalogHighZ PRIVATE NEWRECEIVER ANALOG HIGHZ)
list(APPEND MOENCH_EXECUTABLES moenchAnalogHighZ)


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