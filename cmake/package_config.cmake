# This cmake code creates the configuration that is found and used by
# find_package() of another cmake project

# get lower and upper case project name for the configuration files

# configure and install the configuration files
include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_SOURCE_DIR}/cmake/project-config.cmake.in"
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME_LOWER}-config.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME_LOWER}
  PATH_VARS CMAKE_INSTALL_DIR)

write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME_LOWER}-config-version.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(FILES
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME_LOWER}-config.cmake"
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME_LOWER}-config-version.cmake"
  COMPONENT devel
  DESTINATION ${CMAKE_INSTALL_DIR}
)

if (PROJECT_LIBRARIES OR PROJECT_STATIC_LIBRARIES)
  install(
    EXPORT "${TARGETS_EXPORT_NAME}"
    FILE ${PROJECT_NAME_LOWER}-targets.cmake
    DESTINATION ${CMAKE_INSTALL_DIR}
    )
endif ()
