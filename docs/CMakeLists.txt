# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
find_package(Doxygen REQUIRED)
find_package(Sphinx REQUIRED)




# Utility to generate command line documentation
add_executable(gendoc src/gendoc.cpp)
# This is a bit hacky, but better than exposing stuff? 
target_include_directories(gendoc PRIVATE ${PROJECT_SOURCE_DIR}/slsDetectorSoftware/src)
target_link_libraries(gendoc PRIVATE
  slsDetectorShared
)
set_target_properties(gendoc PROPERTIES 
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)




#Doxygen
set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

#Sphinx
set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR})

set(SPHINX_SOURCE_FILES
  src/commandline.rst
  src/container_utils.rst
  src/consuming.rst
  src/dependencies.rst
  src/detector.rst
  src/index.rst
  src/installation.rst
  src/pydetector.rst
  src/pyenums.rst
  src/pyexamples.rst
  src/servers.rst
  src/receiver_api.rst
  src/result.rst
  src/type_traits.rst
  src/ToString.rst
  src/examples.rst
  src/pygettingstarted.rst
  src/firmware.rst
  src/serverupgrade.rst
  src/virtualserver.rst
  src/serverdefaults.rst
  src/quick_start_guide.rst
  src/troubleshooting.rst
  src/receivers.rst
  src/slsreceiver.rst
  src/udpheader.rst
  src/udpconfig.rst
  src/udpdetspec.rst
  src/fileformat.rst
  src/slsreceiverheaderformat.rst
  src/masterfileattributes.rst
  src/binaryfileformat.rst
  src/hdf5fileformat.rst
  src/zmqjsonheaderformat.rst
)

foreach(filename ${SPHINX_SOURCE_FILES})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${filename} 
              "${SPHINX_BUILD}/${filename}")
endforeach(filename ${SPHINX_SOURCE_FILES})

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in"
  "${SPHINX_BUILD}/conf.py"
  @ONLY)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/gen_server_doc.py.in"
  "${SPHINX_BUILD}/gen_server_doc.py"
  @ONLY)

  configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/static/extra.css"
  "${SPHINX_BUILD}/static/css/extra.css"
  @ONLY)

add_custom_target(server_rst python gen_server_doc.py)  

add_custom_target(docs
              gendoc
              COMMAND python gen_server_doc.py
              COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
              COMMAND ${SPHINX_EXECUTABLE} -a -b html
              -Dbreathe_projects.slsDetectorPackage=${CMAKE_CURRENT_BINARY_DIR}/xml
              -c "${SPHINX_BUILD}"
              ${SPHINX_BUILD}/src 
              ${SPHINX_BUILD}/html
              COMMENT "Generating documentation with Sphinx")

add_custom_target(rst
              COMMAND ${SPHINX_EXECUTABLE} -a -b html
              -Dbreathe_projects.slsDetectorPackage=${CMAKE_CURRENT_BINARY_DIR}/xml
              -c "${SPHINX_BUILD}"
              ${SPHINX_BUILD}/src 
              ${SPHINX_BUILD}/html
              COMMENT "Generating documentation with Sphinx")

