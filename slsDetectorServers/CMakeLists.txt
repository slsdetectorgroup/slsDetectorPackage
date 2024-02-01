# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package



# Install fake the library
install(TARGETS slsProjectCSettings
    EXPORT "${TARGETS_EXPORT_NAME}"
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
add_subdirectory(ctbDetectorServer)
add_subdirectory(xilinx_ctbDetectorServer)
add_subdirectory(eigerDetectorServer)
add_subdirectory(gotthardDetectorServer)
add_subdirectory(jungfrauDetectorServer)
add_subdirectory(mythen3DetectorServer)
add_subdirectory(gotthard2DetectorServer)
add_subdirectory(moenchDetectorServer)
