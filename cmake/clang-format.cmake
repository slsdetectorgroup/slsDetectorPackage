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

#target for formatting soruce files
add_custom_target(format
    COMMENT "Running clang-format to change files"
    COMMAND ${ClangFormat_BIN}
    -style=file
    -i
    ${ALL_SOURCE_FILES}
)


#target to check format on source files 
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

# debug to check which file will be formatted
add_custom_target(
    listformatfiles
    COMMAND
    echo ${ALL_SOURCE_FILES}
)
