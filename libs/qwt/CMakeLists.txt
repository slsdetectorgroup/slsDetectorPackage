#inspired by: https://github.com/ericriff/qwt/blob/master/CMakeLists.txt

# Build a static version of qwt from tar file 
cmake_minimum_required(VERSION 3.12)
project(qwt)
set(PROJECT_VERSION 6.1.5)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# cmake_minimum_required above resets the policies set in the top level file
if (${CMAKE_VERSION} VERSION_GREATER "3.24")
    cmake_policy(SET CMP0135 NEW) #Fetch content download timestamp
endif()

# Extract qwt sources from the supplied source file. FetchContent unpacks
# with the archive support built into CMake, so neither tar nor bzip2 has
# to be installed. The archive has no top level CMakeLists.txt, so nothing
# is added to the build here.
include(FetchContent)
FetchContent_Declare(
    qwt_source
    URL ${CMAKE_CURRENT_SOURCE_DIR}/qwt-6.1.5.tar.bz2
    # Compute hash: md5sum qwt-6.1.5.tar.bz2
    URL_HASH MD5=d65582f99312796ed42c3be3208ed3db
)
FetchContent_MakeAvailable(qwt_source)
set(QWTBASE ${qwt_source_SOURCE_DIR})

# Without this define moc files are not included and linking of examples fails.
add_definitions(-DQWT_MOC_INCLUDE)

find_package(Qt5 REQUIRED
				COMPONENTS
					Core
					Gui
					Widgets
					OpenGL
					Concurrent
					PrintSupport
					Svg
)


file(GLOB QWT_SOURCES ${QWTBASE}/src/*.cpp)
file(GLOB QWT_HEADERS ${QWTBASE}/src/*.h)
add_library(qwt STATIC ${QWT_SOURCES} ${QWT_HEADERS} )
target_include_directories(qwt SYSTEM PUBLIC 
	$<BUILD_INTERFACE:${QWTBASE}/src>
)

target_link_libraries(qwt PUBLIC
							Qt5::Core
							Qt5::Gui
							Qt5::Widgets
							Qt5::OpenGL
							Qt5::Concurrent
							Qt5::PrintSupport
							Qt5::Svg
)


# Disable all warnings in QWT since we don’t want to patch the library
target_compile_options(qwt PRIVATE -w)
