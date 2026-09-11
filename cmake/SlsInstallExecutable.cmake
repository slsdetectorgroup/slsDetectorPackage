# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2026 Contributors to the SLS Detector Package

function(sls_install_executable target)
    cmake_parse_arguments(ARG "" "DESTINATION;COMPONENT;EXPORT" "" ${ARGN})
    if(NOT ARG_DESTINATION)
        set(ARG_DESTINATION "${CMAKE_INSTALL_BINDIR}")
    endif()

    set(install_args DESTINATION "${ARG_DESTINATION}")
    if(ARG_COMPONENT)
        list(APPEND install_args COMPONENT "${ARG_COMPONENT}")
    endif()

    if(SLS_INSTALL_VERSIONED_BINARIES)
        install(PROGRAMS "$<TARGET_FILE:${target}>"
            ${install_args}
            RENAME "$<TARGET_FILE_BASE_NAME:${target}>-${PROJECT_VERSION}${CMAKE_EXECUTABLE_SUFFIX}"
        )
    else()
        set(export_args)
        if(ARG_EXPORT)
            list(APPEND export_args EXPORT "${ARG_EXPORT}")
        endif()
        install(TARGETS ${target}
            ${export_args}
            RUNTIME ${install_args}
        )
    endif()
endfunction()
