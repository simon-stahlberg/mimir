if (BUILD_LIBMIMIR)
    include(CMakePackageConfigHelpers)
    include(GNUInstallDirs)

    add_library(
            mimir
            STATIC
            lib/mimir.cpp
            ${MIMIR_SRC_FILES}
    )

    target_include_directories(mimir PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
    target_link_libraries(
            mimir
            PUBLIC
            project_options
            project_warnings
            boost::boost
    )

    install(TARGETS
            mimir
            # the project_options/warnings are required to be exported as well
            project_options
            project_warnings
            EXPORT mimirTargets
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(
            DIRECTORY
            include/mimir
            DESTINATION
            ${CMAKE_INSTALL_INCLUDEDIR}
    )

    set(PKGCONFIG ${CMAKE_CURRENT_BINARY_DIR}/mimir.pc)

    configure_file(cmake/in/mimir.pc.in ${PKGCONFIG} @ONLY)

    install(FILES ${PKGCONFIG} DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

    write_basic_package_version_file(
            "${PROJECT_BINARY_DIR}/mimirConfigVersion.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY AnyNewerVersion
    )

    configure_package_config_file(
            "cmake/in/mimirConfig.cmake.in"
            "${PROJECT_BINARY_DIR}/mimirConfig.cmake"
            INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mimir
    )


    install(FILES
            "${PROJECT_BINARY_DIR}/mimirConfigVersion.cmake"
            "${PROJECT_BINARY_DIR}/mimirConfig.cmake"
            DESTINATION
            ${CMAKE_INSTALL_LIBDIR}/cmake/mimir
    )

    install(
            EXPORT mimirTargets
            FILE mimirTargets.cmake
            NAMESPACE mimir::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mimir/
    )

endif ()