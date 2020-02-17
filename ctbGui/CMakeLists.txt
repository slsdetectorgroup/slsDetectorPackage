

find_package(ROOT CONFIG REQUIRED  COMPONENTS Core Gui)
find_package(TIFF REQUIRED)

target_include_directories(ROOT::Core INTERFACE "${ROOT_INCLUDE_DIRS}")
add_library(ROOT::Flags_CXX IMPORTED INTERFACE)
separate_arguments(ROOT_CXX_FLAGS)
target_compile_options(ROOT::Flags_CXX INTERFACE ${ROOT_CXX_FLAGS})
separate_arguments(ROOT_DEFINITIONS)
target_compile_definitions(ROOT::Flags_CXX INTERFACE ${ROOT_DEFINITIONS})

# This fixes a bug in the linker flags
string(REPLACE "-L " "-L" ROOT_EXE_LINKER_FLAGS "${ROOT_EXE_LINKER_FLAGS}")
separate_arguments(ROOT_EXE_LINKER_FLAGS)

# Stuck into using old property method due to separate -L and -l arguments
# (A full path to -l is better!)
set_property(TARGET ROOT::Flags_CXX PROPERTY
    INTERFACE_LINK_LIBRARIES ${ROOT_EXE_LINKER_FLAGS})
set_property(TARGET ROOT::Core PROPERTY
    INTERFACE_INCLUDE_DIRECTORIES "${ROOT_INCLUDE_DIRS}")


add_executable(ctbGui 
    ctbGui.cpp 
    ctbMain.cpp
    ctbDacs.cpp
    ctbPowers.cpp
    ctbSlowAdcs.cpp
    ctbSignals.cpp
    ctbAdcs.cpp
    ctbPattern.cpp
    ctbAcquisition.cpp
    ${CMAKE_SOURCE_DIR}/slsDetectorCalibration/tiffIO.cpp
)


#TODO! Replace with target
target_include_directories(ctbGui PRIVATE 
    ${CMAKE_SOURCE_DIR}/slsDetectorCalibration/dataStructures
    ${CMAKE_SOURCE_DIR}/slsDetectorCalibration/interpolations
    ${CMAKE_SOURCE_DIR}/slsDetectorCalibration/
)

# Headders needed for ROOT dictionary generation
set( HEADERS
    ctbDefs.h
    ctbMain.h 
    ctbDacs.h   
    ctbPattern.h  
    ctbSignals.h  
    ctbAdcs.h 
    ctbAcquisition.h 
    ctbPowers.h   
    ctbSlowAdcs.h
)

#set(ROOT_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# ROOT dictionary generation 
include("${ROOT_DIR}/modules/RootNewMacros.cmake")
root_generate_dictionary(ctbDict ${HEADERS} LINKDEF ctbLinkDef.h)
add_library(ctbRootLib SHARED ctbDict.cxx)
target_include_directories(ctbRootLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ctbRootLib PUBLIC 
    ROOT::Core 
    slsDetectorShared
    slsSupportLib
    ${ROOT_LIBRARIES} 
    ${ROOT_EXE_LINKER_FLAGS}
)

set_target_properties(
    ctbRootLib PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

target_link_libraries(ctbGui PUBLIC 
    slsDetectorShared
    slsSupportLib
    ctbRootLib
    ${TIFF_LIBRARIES}
)

set_target_properties(ctbGui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin

)