
macro(configure_ccache)
    # If it's available, use ccache to cache compilation results. The two ccache options
    # allow sharing compilation results between different build directories.
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND AND NOT WIN32)  # Windows Github Actions find "ccache" --> ignore it.
        message("Using ccache")
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE
            "CCACHE_BASEDIR=${CMAKE_CURRENT_SOURCE_DIR} CCACHE_NOHASHDIR=true ccache")
    endif(CCACHE_FOUND AND NOT WIN32)
endmacro()
