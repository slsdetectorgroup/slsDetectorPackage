

find_package(ROOT CONFIG REQUIRED  COMPONENTS Core Gui)

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
)

#TODO! Replace with target
target_include_directories(ctbGui PRIVATE 
    ${CMAKE_SOURCE_DIR}/slsDetectorCalibration/dataStructures
    ${CMAKE_SOURCE_DIR}/slsDetectorCalibration/interpolations
    ${CMAKE_SOURCE_DIR}/slsDetectorCalibration/
)

target_link_libraries(ctbGui PUBLIC 
    ROOT::Core
    ${ROOT_LIBRARIES} 
    ${ROOT_EXE_LINKER_FLAGS}
    slsDetectorShared
)


set_target_properties(ctbGui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)