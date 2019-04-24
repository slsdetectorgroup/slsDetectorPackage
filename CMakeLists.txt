cmake_minimum_required(VERSION 3.9)
project(slsDetectorPackage)
set(PROJECT_VERSION 5.0.0)

include(CheckIPOSupported)




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
option (SLS_USE_MYTHEN302_CTB_RECEIVER "Mythen302 CTB Receiver" OFF)
option (SLS_USE_TESTS "TESTS" ON)
option (SLS_USE_INTEGRATION_TESTS "Integration Tests" ON)
option(SLS_USE_SANITIZER "Sanitizers for debugging" OFF)
option(SLS_USE_PYTHON "Python bindings" OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (default Release)" FORCE)
endif()


#Add two fake libraries to manage options
add_library(slsProjectOptions INTERFACE)
add_library(slsProjectWarnings INTERFACE)
target_compile_features(slsProjectOptions INTERFACE cxx_std_11)
target_compile_options(slsProjectWarnings INTERFACE 
                                            -Wall
                                            -Wextra
                                            -Wno-unused-parameter #Needs to be slowly mitigated
                                            # -Wold-style-cast
                                            -Wnon-virtual-dtor
                                            -Woverloaded-virtual
                                            -Wdouble-promotion
                                            -Wformat=2
                                            -Wredundant-decls
                                            # -Wconversion
                                            -Wdouble-promotion

                                 )


#Testing for minimum version for compilers
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.2)
        message(FATAL_ERROR "Clang version must be at least 3.2!")
    endif()
    target_compile_options(slsProjectWarnings INTERFACE -Wshadow) #Clag does not warn on constructor
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
            message(FATAL_ERROR "GCC version must be at least 4.8!")
    endif()

    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5)
            target_compile_options(slsProjectWarnings INTERFACE 
                                             -Wno-missing-field-initializers)
    endif()
    
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6.0)
        target_compile_options(slsProjectWarnings INTERFACE 
                                                -Wno-misleading-indentation # mostly in rapidjson remove using clang format
                                                -Wduplicated-cond
                                                -Wnull-dereference )

    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 8.0)
        target_compile_options(slsProjectWarnings INTERFACE 
                                                -Wno-class-memaccess )

    endif()
endif()


if(SLS_USE_SANITIZER)
    target_compile_options(slsProjectOptions INTERFACE -fsanitize=address,undefined)
    target_link_libraries(slsProjectOptions INTERFACE -fsanitize=address,undefined)
    # target_compile_options(slsProjectOptions INTERFACE -fsanitize=thread)
    # target_link_libraries(slsProjectOptions INTERFACE -fsanitize=thread)

    endif()


# Install fake the library
install(TARGETS slsProjectOptions slsProjectWarnings
    EXPORT "${TARGETS_EXPORT_NAME}"
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INSTALL_RPATH "$ORIGIN")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)



find_package(Doxygen)
find_package(ZeroMQ 4 REQUIRED)

if (SLS_USE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif(SLS_USE_TESTS)

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



if (SLS_USE_INTEGRATION_TESTS)
    add_subdirectory(integrationTests)
endif (SLS_USE_INTEGRATION_TESTS)

if (SLS_USE_PYTHON)
    add_subdirectory(python)
endif(SLS_USE_PYTHON)

configure_file( .clang-tidy  
    ${CMAKE_BINARY_DIR}/.clang-tidy  )

if(SLS_MASTER_PROJECT)
    # Set install dir CMake packages
    set(CMAKE_INSTALL_DIR ${CMAKE_INSTALL_DATADIR}/cmake/sls)
    # Set the list of exported targets
    set(PROJECT_LIBRARIES slsSupportLib slsDetectorShared slsReceiverShared)
    # Generate and install package config file and version
    include(cmake/package_config.cmake)
endif()
