function(default_build_type val)
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE ${val} CACHE STRING "Build type (default ${val})" FORCE)
endif()
endfunction()

function(set_std_fs_lib)
# from pybind11
# Check if we need to add -lstdc++fs or -lc++fs or nothing
if(DEFINED CMAKE_CXX_STANDARD AND CMAKE_CXX_STANDARD LESS 17)
  set(STD_FS_NO_LIB_NEEDED TRUE)
elseif(MSVC)
  set(STD_FS_NO_LIB_NEEDED TRUE)
else()
  file(
    WRITE ${CMAKE_CURRENT_BINARY_DIR}/main.cpp
    "#include <filesystem>\nint main(int argc, char ** argv) {\n  std::filesystem::path p(argv[0]);\n  return p.string().length();\n}"
  )
  try_compile(
    STD_FS_NO_LIB_NEEDED ${CMAKE_CURRENT_BINARY_DIR}
    SOURCES ${CMAKE_CURRENT_BINARY_DIR}/main.cpp
    COMPILE_DEFINITIONS -std=c++17)
  try_compile(
    STD_FS_NEEDS_STDCXXFS ${CMAKE_CURRENT_BINARY_DIR}
    SOURCES ${CMAKE_CURRENT_BINARY_DIR}/main.cpp
    COMPILE_DEFINITIONS -std=c++17
    LINK_LIBRARIES stdc++fs)
  try_compile(
    STD_FS_NEEDS_CXXFS ${CMAKE_CURRENT_BINARY_DIR}
    SOURCES ${CMAKE_CURRENT_BINARY_DIR}/main.cpp
    COMPILE_DEFINITIONS -std=c++17
    LINK_LIBRARIES c++fs)
endif()

if(${STD_FS_NEEDS_STDCXXFS})
  set(STD_FS_LIB stdc++fs PARENT_SCOPE)
elseif(${STD_FS_NEEDS_CXXFS})
  set(STD_FS_LIB c++fs PARENT_SCOPE)
elseif(${STD_FS_NO_LIB_NEEDED})
  set(STD_FS_LIB "" PARENT_SCOPE)
else()
  message(WARNING "Unknown C++17 compiler - not passing -lstdc++fs")
  set(STD_FS_LIB "")
endif()
endfunction()