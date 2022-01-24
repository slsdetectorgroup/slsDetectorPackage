find_package(TIFF REQUIRED)
add_library(tiffio STATIC tiffIO.cpp)
target_include_directories(tiffio PUBLIC include/)
target_link_libraries(tiffio PUBLIC TIFF::TIFF)