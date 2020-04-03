
add_library(slsProjectCWarnings INTERFACE)
target_compile_options(slsProjectCWarnings INTERFACE 
                                            -Wall
                                            -Wextra
                                            -Wno-unused-parameter #Needs to be slowly mitigated
                                            -Wdouble-promotion
                                            -Wformat=2
                                            -Wredundant-decls
                                            # -Wconversion
                                            -Wdouble-promotion
                                            -Werror=return-type
                                 )

# Install fake the library
install(TARGETS slsProjectCWarnings
    EXPORT "${TARGETS_EXPORT_NAME}"
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
#add_subdirectory(ctbDetectorServer)
#add_subdirectory(eigerDetectorServer)
#add_subdirectory(gotthardDetectorServer)
#add_subdirectory(jungfrauDetectorServer)
add_subdirectory(mythen3DetectorServer)
#add_subdirectory(gotthard2DetectorServer)
#add_subdirectory(moenchDetectorServer)
