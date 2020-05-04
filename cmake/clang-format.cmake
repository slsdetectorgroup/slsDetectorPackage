# A CMake script to find all source files and setup clang-format targets for them

# Find all source files
set(ClangFormat_CXX_FILE_EXTENSIONS ${ClangFormat_CXX_FILE_EXTENSIONS} *.cpp *.h *.cxx *.hxx *.hpp *.cc *.ipp)
file(GLOB_RECURSE ALL_SOURCE_FILES ${ClangFormat_CXX_FILE_EXTENSIONS})

# Don't include some common build folders
set(ClangFormat_EXCLUDE_PATTERNS ${ClangFormat_EXCLUDE_PATTERNS} "/CMakeFiles/" "cmake")

# get all project files file
foreach (SOURCE_FILE ${ALL_SOURCE_FILES}) 
    foreach (EXCLUDE_PATTERN ${ClangFormat_EXCLUDE_PATTERNS})
        string(FIND ${SOURCE_FILE} ${EXCLUDE_PATTERN} EXCLUDE_FOUND) 
        if (NOT ${EXCLUDE_FOUND} EQUAL -1) 
            list(REMOVE_ITEM ALL_SOURCE_FILES ${SOURCE_FILE})
        endif () 
    endforeach ()
endforeach ()

add_custom_target(format
    COMMENT "Running clang-format to change files"
    COMMAND ${ClangFormat_BIN}
    -style=file
    -i
    ${ALL_SOURCE_FILES}
)

#put back i 
add_custom_target(format-check
    COMMENT "Checking clang-format changes"
    # Use ! to negate the result for correct output
    COMMAND !
    ${ClangFormat_BIN}
    -style=file
    -output-replacements-xml
    ${ALL_SOURCE_FILES}
    | grep -q "replacement offset" 
)


add_custom_target(
    sf
    COMMAND
    echo ${ALL_SOURCE_FILES}
)



add_custom_target( list
COMMAND     
    foreach (SOURCE_FILE ${ALL_SOURCE_FILES}) 
        message(${SOURCE_FILE})
    endforeach ()
)

# # Get the path to this file
# get_filename_component(_clangcheckpath ${CMAKE_CURRENT_LIST_FILE} PATH)
# # have at least one here by default
# set(CHANGED_FILE_EXTENSIONS ".cpp")
# foreach(EXTENSION ${ClangFormat_CXX_FILE_EXTENSIONS})
#     set(CHANGED_FILE_EXTENSIONS "${CHANGED_FILE_EXTENSIONS},${EXTENSION}" )
# endforeach()

# set(EXCLUDE_PATTERN_ARGS)
# foreach(EXCLUDE_PATTERN ${ClangFormat_EXCLUDE_PATTERNS})
#     list(APPEND EXCLUDE_PATTERN_ARGS "--exclude=${EXCLUDE_PATTERN}")
# endforeach()

# # call the script to check changed files in git
# add_custom_target(format-check-changed
#     COMMENT "Checking changed files in git"
#     WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#     COMMAND ${_clangcheckpath}/../scripts/clang-format-check-changed.py 
#     --file-extensions \"${CHANGED_FILE_EXTENSIONS}\"
#     ${EXCLUDE_PATTERN_ARGS}
#     --clang-format-bin ${ClangFormat_BIN}
# )

