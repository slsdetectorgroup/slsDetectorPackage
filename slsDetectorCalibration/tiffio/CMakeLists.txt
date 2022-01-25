find_package(TIFF REQUIRED)
add_library(tiffio STATIC src/tiffIO.cpp)
target_include_directories(tiffio PUBLIC include/)
target_link_libraries(tiffio 
    PUBLIC 
        TIFF::TIFF 
    PRIVATE 
        slsProjectWarnings
        slsProjectOptions
)

if(SLS_USE_TESTS)
    add_subdirectory(tests)
endif()