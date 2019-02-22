cmake_minimum_required(VERSION 2.8)
# cmake_minimum_required(VERSION 3.5)
project(slsDetectorPackage)
include(cmake/project_version.cmake)

# Include additional modules that are used unconditionally
include(GNUInstallDirs)

# If conda build, always set lib dir to 'lib'
if($ENV{CONDA_BUILD})
  set(CMAKE_INSTALL_LIBDIR "lib")
endif()

# Set lower / upper case project names
string(TOUPPER "${PROJECT_NAME}" PROJECT_NAME_UPPER)
string(TOLOWER "${PROJECT_NAME}" PROJECT_NAME_LOWER)

# Set targets export name (used by slsDetectorPackage and dependencies)
set(TARGETS_EXPORT_NAME "${PROJECT_NAME_LOWER}-targets")
#set(namespace "${PROJECT_NAME}::")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})


# Check if project is being used directly or via add_subdirectory
set(SLS_MASTER_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(SLS_MASTER_PROJECT ON)
endif()



option (SLS_USE_HDF5 "HDF5 File format" OFF)
option (SLS_USE_TEXTCLIENT "Text Client" OFF)
option (SLS_USE_RECEIVER "Receiver" OFF)
option (SLS_USE_GUI "GUI" OFF)
option (SLS_USE_TESTS "TESTS" ON)
option (SLS_USE_INTEGRATION_TESTS "Integration Tests" ON)



#Testing for minimum version for compilers
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # clang does not support -Wno-misleading-indentation 
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.2)
        message(FATAL_ERROR "Clang version must be at least 3.2!")
    endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
            message(FATAL_ERROR "GCC version must be at least 4.8!")
    endif()
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-misleading-indentation -Wno-stringop-truncation -Wno-class-memaccess")
endif()

# Suppressed warnings in GCC
# -Wno-stringop-truncation happens when we do sls::strcpy_safe truncates string instead of overwrite buffer
# -Wno-class-memaccess - on memcpy in rapidjson should be investigated upgrade implementation?
# -Wno-misleading-indentation - until clang format

#set (CMAKE_CXX_STANDARD 11)
#set (CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 ")

# set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=thread")
# set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=thread")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address,undefined")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address,undefiend")



set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INSTALL_RPATH "$ORIGIN")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)



find_package(Doxygen)
find_package(ZeroMQ 4 REQUIRED)


# Support library containing functionallity common to 
# detector and receiver
add_subdirectory(slsSupportLib)

if (SLS_USE_TEXTCLIENT)
    add_subdirectory(slsDetectorSoftware)
endif (SLS_USE_TEXTCLIENT)

if (SLS_USE_RECEIVER)
    if (SLS_USE_HDF5)
        find_package(HDF5 1.10 COMPONENTS CXX REQUIRED)
    endif (SLS_USE_HDF5)
    add_subdirectory(slsReceiverSoftware)
    add_subdirectory(manual/manual-api)    
endif (SLS_USE_RECEIVER)

if (SLS_USE_GUI)   
    find_package(Qt4 REQUIRED)
    find_package(Qwt 6 REQUIRED)
    if (QT4_FOUND AND QWT_FOUND)
        add_subdirectory(slsDetectorGui)
    endif()
endif (SLS_USE_GUI)

if (SLS_USE_TESTS)
    enable_testing()
endif(SLS_USE_TESTS)

if (SLS_USE_INTEGRATION_TESTS)
    add_subdirectory(integrationTests)
endif (SLS_USE_INTEGRATION_TESTS)




if(SLS_MASTER_PROJECT)
    # Set install dir CMake packages
    set(CMAKE_INSTALL_DIR ${CMAKE_INSTALL_DATADIR}/cmake/sls)
    # Set the list of exported targets
    set(PROJECT_LIBRARIES slsSupportLib slsDetectorShared slsReceiverShared)
    # Generate and install package config file and version
    include(cmake/package_config.cmake)
endif()

