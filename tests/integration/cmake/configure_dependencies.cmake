macro(configure_dependencies list_of_dependencies)
    # We installed the dependencies into the subdirectories under the install prefix.
    # Hence must append them to the single cmake_prefix_path.
    message("Configure dependencies of Loki:")
    message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
    list(LENGTH CMAKE_PREFIX_PATH PREFIX_PATH_COUNT)
    if(PREFIX_PATH_COUNT GREATER 1)
        message(FATAL_ERROR "Only one prefix path is allowed. Found multiple paths in CMAKE_PREFIX_PATH. Please add dependencies to the CMake Superbuild.")
    endif()

    # Assuming there's only one path in CMAKE_PREFIX_PATH, get that path
    list(GET CMAKE_PREFIX_PATH 0 SINGLE_CMAKE_PREFIX_PATH)

    # Clear MODIFIED_CMAKE_PREFIX_PATH before appending
    set(MODIFIED_CMAKE_PREFIX_PATH "")

    # Iterate over list of names and append each to SINGLE_CMAKE_PREFIX_PATH
    foreach(DEPENDENCY_NAME ${list_of_dependencies})
        list(APPEND MODIFIED_CMAKE_PREFIX_PATH "${SINGLE_CMAKE_PREFIX_PATH}/${DEPENDENCY_NAME}")
    endforeach()

   #  message(${MODIFIED_CMAKE_PREFIX_PATH})
    set(CMAKE_PREFIX_PATH ${MODIFIED_CMAKE_PREFIX_PATH})
    message(STATUS "MODIFIED_CMAKE_PREFIX_PATH:")
    foreach(CMAKE_PREFIX_PATH_ARG ${CMAKE_PREFIX_PATH})
        message(STATUS "-- ${CMAKE_PREFIX_PATH_ARG}")
    endforeach()
    # set(CMAKE_FIND_DEBUG_MODE ON)
endmacro()