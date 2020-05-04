# Find Clang format
# 
# 
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
	# A CMake script to find all source files and setup clang-format targets for them
	include(clang-format)
else()
    message("clang-format not found. Not setting up format targets")
endif()
