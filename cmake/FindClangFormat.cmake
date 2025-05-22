# Find Clang format
if(NOT ClangFormat_BIN_NAME)
	set(ClangFormat_BIN_NAME clang-format)
endif()

# if custom path check there first
if(ClangFormat_ROOT_DIR)
    find_program(ClangFormat_BIN 
        NAMES
        ${ClangFormat_BIN_NAME}
        PATHS
        "${ClangFormat_ROOT_DIR}"
        NO_DEFAULT_PATH)
endif()

find_program(ClangFormat_BIN NAMES ${ClangFormat_BIN_NAME})

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    ClangFormat
    DEFAULT_MSG 
    ClangFormat_BIN)

mark_as_advanced(
    ClangFormat_BIN)

if(ClangFormat_FOUND)
    execute_process(COMMAND ${ClangFormat_BIN} --version
                 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                 OUTPUT_VARIABLE CLANG_VERSION_TEXT)
    string(REGEX MATCH "([0-9]+)\\.[0-9]+\\.[0-9]+" CLANG_VERSION ${CLANG_VERSION_TEXT})
    if((${CLANG_VERSION} GREATER "9") OR (${CLANG_VERSION} EQUAL "9"))
        # A CMake script to find all source files and setup clang-format targets for them
        message(STATUS "found clang-format \"${CLANG_VERSION}\" adding formatting targets")
        include(clang-format)
    else()
        message(STATUS "clang-format version \"${CLANG_VERSION}\" found but need at least 9. Not setting up format targets")
    endif()
else()
    message(STATUS "clang-format not found. Not setting up format targets")
endif()
