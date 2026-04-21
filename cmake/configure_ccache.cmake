
macro(configure_ccache)
    # If it's available, use ccache to cache compilation results. The two ccache options
    # allow sharing compilation results between different build directories.
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM AND NOT WIN32)  # Windows Github Actions find "ccache" --> ignore it.
        set(_ccache_launcher
            "${CMAKE_COMMAND}"
            -E
            env
            "CCACHE_BASEDIR=${PROJECT_SOURCE_DIR}"
            "CCACHE_NOHASHDIR=true"
            "${CCACHE_PROGRAM}"
        )

        if(CMAKE_C_COMPILER AND NOT CMAKE_C_COMPILER_LAUNCHER)
            set(CMAKE_C_COMPILER_LAUNCHER "${_ccache_launcher}" CACHE STRING "C compiler launcher")
        endif()

        if(CMAKE_CXX_COMPILER AND NOT CMAKE_CXX_COMPILER_LAUNCHER)
            set(CMAKE_CXX_COMPILER_LAUNCHER "${_ccache_launcher}" CACHE STRING "CXX compiler launcher")
        endif()

        message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
    endif()
endmacro()
