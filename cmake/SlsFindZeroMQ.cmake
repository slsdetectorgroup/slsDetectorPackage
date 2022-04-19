function(custom_find_zmq)
    set(ZeroMQ_HINT "" CACHE STRING "Hint where ZeroMQ could be found")
    #Adapted from: https://github.com/zeromq/cppzmq/
    if (NOT TARGET libzmq)
        if(ZeroMQ_HINT)
            message(STATUS "Looking for ZeroMQ in: ${ZeroMQ_HINT}")
            find_package(ZeroMQ 4 
                NO_DEFAULT_PATH
                HINTS ${ZeroMQ_DIR}
            )
        else()
            find_package(ZeroMQ 4 QUIET)
        endif()
        
    # libzmq autotools install: fallback to pkg-config
    if(NOT ZeroMQ_FOUND)
        message(STATUS "CMake libzmq package not found, trying again with pkg-config (normal install of zeromq)")
        list (APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake/libzmq-pkg-config)
        find_package(ZeroMQ 4 REQUIRED)
    endif()

    # TODO "REQUIRED" above should already cause a fatal failure if not found, but this doesn't seem to work
    if(NOT ZeroMQ_FOUND)
        message(FATAL_ERROR "ZeroMQ was not found, neither as a CMake package nor via pkg-config")
    endif()

    if (ZeroMQ_FOUND AND NOT TARGET libzmq)
        message(FATAL_ERROR "ZeroMQ version not supported!")
    endif()
    endif()

    get_target_property(VAR libzmq IMPORTED_LOCATION)
    message(STATUS "Using zmqlib: ${VAR}")


endfunction()