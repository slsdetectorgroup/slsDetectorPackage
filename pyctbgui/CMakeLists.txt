#cmake_minimum_required(VERSION 3.18)
#project(pyctbgui VERSION 2023.8.17 LANGUAGES C)

# Find Python and NumPy
find_package(Python COMPONENTS Interpreter Development.Module NumPy REQUIRED)

# Create the C extension module
Python_add_library(_decoder MODULE WITH_SOABI  
    src/decoder.c
    src/pm_decode.c
)

# Set properties
target_include_directories(_decoder PRIVATE
    ${Python_NumPy_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(_decoder PRIVATE
    -std=c99
    -Wall
    -Wextra
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/pyctbgui
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set_target_properties(_decoder PROPERTIES
    OUTPUT_NAME "_decoder"
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/pyctbgui 
)

# Create the main GUI script
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CtbGui  
    ${CMAKE_CURRENT_BINARY_DIR}/CtbGui
)

# Install the executable GUI script into bin)
#install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/CtbGui
#    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
#    COMPONENT python/CtbGui
#)


# Install Python package files (includes subdirectories like ui/)
#install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/pyctbgui/
#   DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/pyctbgui
#    FILES_MATCHING 
#    PATTERN "*.py"
#    PATTERN "*.ui"
#    PATTERN "*.qrc"
#    PATTERN "__pycache__" EXCLUDE
#)

# Copy all Python files (including subdirectories) to build/pyctbgui
#file(GLOB_RECURSE PYCTBGUI_PY_FILES
#    "${CMAKE_CURRENT_SOURCE_DIR}/pyctbgui/*.py"
#    "${CMAKE_CURRENT_SOURCE_DIR}/pyctbgui/**/*.py"
#)

#foreach(pyfile ${PYCTBGUI_PY_FILES})
    # Get relative path
 #   file(RELATIVE_PATH rel_pyfile "${CMAKE_CURRENT_SOURCE_DIR}/pyctbgui" "${pyfile}")
 #   configure_file(${pyfile} ${CMAKE_CURRENT_BINARY_DIR}/pyctbgui/${rel_pyfile} COPYONLY)
#endforeach()