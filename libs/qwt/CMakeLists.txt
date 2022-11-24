#inspired by: https://github.com/ericriff/qwt/blob/master/CMakeLists.txt

# Build a static version of qwt from tar file 
cmake_minimum_required(VERSION 3.12)
project(qwt)
set(PROJECT_VERSION 6.1.5)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Extract qwt sources from the supplied source file
set(INTERNALQWTDIR "qwt-source/")
set(QWTBASE ${CMAKE_CURRENT_BINARY_DIR}/${INTERNALQWTDIR})
file(MAKE_DIRECTORY ${QWTBASE})
execute_process(
	COMMAND tar -xf ${CMAKE_CURRENT_SOURCE_DIR}/qwt-6.1.5.tar.bz2 -C ${QWTBASE} --strip-components=1
	RESULT_VARIABLE ret
)
if(ret AND NOT ret EQUAL 0)
    message( FATAL_ERROR "Could not unpack qwt")
endif()

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


file(GLOB QWT_SOURCES ${QWTBASE}src/*.cpp)
file(GLOB QWT_HEADERS ${QWTBASE}src/*.h)
add_library(qwt STATIC ${QWT_SOURCES} ${QWT_HEADERS} )
target_include_directories(qwt PUBLIC 
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