

find_package(ROOT CONFIG REQUIRED  COMPONENTS Core Gui)
find_package(TIFF REQUIRED)
set( HEADERS
    ctbMain.h 
    ctbDacs.h   
    ctbPattern.h  
    ctbSignals.h  
    ctbAdcs.h 
    ctbAcquisition.h 
    ctbPowers.h   
    ctbSlowAdcs.h
)

include("${ROOT_DIR}/modules/RootNewMacros.cmake")
root_generate_dictionary(ctbDict ${HEADERS} LINKDEF ctbLinkDef.h)
add_library(ctbshared SHARED ctbDict.cxx)
# root_generate_dictionary(G__Example Example.h LINKDEF ctbLinkDef.h)
# ROOT_GENERATE_DICTIONARY( dictionary headerfiles ... [STAGE1] 
#                          LINKDEF linkdeffiles ...
#                          [MODULE module] [DEPENDENCIES dep1 dep2 ...]
#                          [OPTIONS opt1 opt2 ...] )

# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
# ROOT_GENERATE_DICTIONARY(G__Event Event.h LINKDEF EventLinkDef.h)

# #---Create a shared library with geneated dictionary
# add_library(Event SHARED Event.cxx G__Event.cxx)
# target_link_libraries(Event ${ROOT_LIBRARIES})

# #---Create  a main program using the library
# add_executable(Main MainEvent.cxx)
# target_link_libraries(Main Event)

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


target_link_libraries(ctbshared PUBLIC ROOT::Core )

target_link_libraries(ctbGui PUBLIC 
    ROOT::Core
    ${ROOT_LIBRARIES} 
    ${ROOT_EXE_LINKER_FLAGS}
    slsDetectorShared
    ctbshared
    ${TIFF_LIBRARIES}
)


set_target_properties(ctbGui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)