# use (i.e. don't skip) the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH FALSE)

# when building, don't use the install RPATH already (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

# add the automatically determined parts of the RPATH which point to directories outside the build tree to the install
# RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# the RPATH to be used when installing, but only if it's not a system directory
list(
    FIND
    CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
    "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}"
    isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
endif("${isSystemDir}" STREQUAL "-1")

if(INSTALL_PYMODULE)
    message("Configuring installation for python module.")
    if(APPLE)
        set(rpath_orig "'@executable_path'")
    else()
        set(rpath_orig "'$ORIGIN'")
    endif()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath=$ORIGIN")

    install(TARGETS ${reinforce_pymodule} LIBRARY DESTINATION reinforce)
else()
    message("Configuring Installation For C++ Library.")
    #
    # Install pkg-config file
    #

    set(REINFORCE_PKGCONFIG ${CMAKE_CURRENT_BINARY_DIR}/reinforce.pc)

    configure_file(${_cmake_DIR}/in/reinforce.pc.in ${REINFORCE_PKGCONFIG} @ONLY)

    install(FILES ${REINFORCE_PKGCONFIG} DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

    write_basic_package_version_file(
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME_LOWERCASE}ConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion)

    configure_package_config_file(
        "${_cmake_DIR}/in/${PROJECT_NAME_LOWERCASE}Config.cmake.in"
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME_LOWERCASE}Config.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME_LOWERCASE}/cmake/)

    # CMake requires all interface dependencies of our library targets, including options and warnings, to be exported.
    install(TARGETS project_options EXPORT ${PROJECT_NAME_LOWERCASE}Options)
    install(TARGETS project_warnings EXPORT ${PROJECT_NAME_LOWERCASE}Warnings)
    install(
        TARGETS ${reinforce_lib}
        EXPORT ${PROJECT_NAME_LOWERCASE}Targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
        BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Runtime
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT Development)

    if(ENABLE_BUILD_PYTHON_EXTENSION)
        # to use the latest build of the library for a development package install (pip install path/to/project -e), we
        # configure an optional install component of the python extension in-source
        install(
            TARGETS ${reinforce_pymodule}
            LIBRARY DESTINATION ${PROJECT_PYREINFORCE_DIR}
                    COMPONENT PyExtension_inplace
                    OPTIONAL)
        # this target can be called conveniently from within IDEs to replace the current build of a development install
        add_custom_target(
            install_${reinforce_pymodule}_insource
            COMMAND ${CMAKE_COMMAND} --install . --component PyExtension_inplace
            DEPENDS ${reinforce_pymodule})
    endif()

    install(
        EXPORT ${PROJECT_NAME_LOWERCASE}Options
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME_LOWERCASE}/cmake/
        NAMESPACE ${PROJECT_NAME_LOWERCASE}::)
    install(
        EXPORT ${PROJECT_NAME_LOWERCASE}Warnings
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME_LOWERCASE}/cmake/
        NAMESPACE ${PROJECT_NAME_LOWERCASE}::)
    install(
        EXPORT ${PROJECT_NAME_LOWERCASE}Targets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME_LOWERCASE}/cmake/
        NAMESPACE ${PROJECT_NAME_LOWERCASE}::)

    install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME_LOWERCASE}ConfigVersion.cmake"
                  "${PROJECT_BINARY_DIR}/${PROJECT_NAME_LOWERCASE}Config.cmake"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME_LOWERCASE}/cmake/)
    # this installation assumes that the project has an eponymous include directory for the project c++ library we
    # install that sub-directory instead of PROJECT_reinforce_INCLUDE_DIR, in order to avoid an include/include/proj_name
    # situation and get the correct include/proj_name.
    install(DIRECTORY ${PROJECT_REINFORCE_INCLUDE_DIR}/${PROJECT_NAME_LOWERCASE} DESTINATION include)

    export(PACKAGE reinforce)

endif()
