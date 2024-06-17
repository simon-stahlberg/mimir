
find_path(NAUTY_INCLUDE_DIR NAMES nauty.h PATHS "${CMAKE_PREFIX_PATH}/include")
find_library(NAUTY_LIBRARY NAMES nauty PATHS "${CMAKE_PREFIX_PATH}/lib")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Nauty DEFAULT_MSG NAUTY_LIBRARY NAUTY_INCLUDE_DIR)

if(NAUTY_FOUND)
    add_library(Nauty::Nauty UNKNOWN IMPORTED)
    set_target_properties(Nauty::Nauty PROPERTIES
        IMPORTED_LOCATION "${NAUTY_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${NAUTY_INCLUDE_DIR}")
endif()
