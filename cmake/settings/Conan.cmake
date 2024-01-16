macro(run_conan)
    # Download automatically
    set(CONAN_VERSION 0.18.1)
    if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
        message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
        file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/${CONAN_VERSION}/conan.cmake"
                "${CMAKE_BINARY_DIR}/conan.cmake" TLS_VERIFY ON)
    endif ()

    include(${CMAKE_BINARY_DIR}/conan.cmake)
    message("Given Conan Path: ${CONAN_PATH}")
    if (DEFINED CONAN_PATH)
        find_program(CONAN_CMD conan REQUIRED PATHS ${CONAN_PATH} NO_DEFAULT_PATH)
    else ()
        find_program(CONAN_CMD conan REQUIRED)
    endif ()
    message("Found conan: ${CONAN_CMD}")
    execute_process(COMMAND ${CONAN_CMD} "--version")

    #    conan_cmake_autodetect(settings)
    #    conan_cmake_install(PATH_OR_REFERENCE ${CMAKE_CURRENT_SOURCE_DIR}/${DEPENDENCY_DIR}/${CONANFILE}
    #            GENERATOR CMakeDeps CMakeToolchain
    #            OUTPUT_FOLDER ${CMAKE_BINARY_DIR}
    #            SETTINGS ${settings})
    conan_cmake_run(
            CONANFILE
            ${DEPENDENCY_DIR}/${CONANFILE}
            CONAN_COMMAND
            ${CONAN_CMD}
            ${CONAN_EXTRA_REQUIRES}
            OPTIONS
            OUTPUT_FOLDER ${CMAKE_BINARY_DIR}
            ${CONAN_EXTRA_OPTIONS}
            BASIC_SETUP
            CMAKE_TARGETS # individual targets to link to
            KEEP_RPATHS
            BUILD missing
            PROFILE default
            PROFILE_BUILD default
            PROFILE_AUTO ALL # ALL means that all the settings are taken from CMake's detection
    )
    include(${PROJECT_BINARY_DIR}/conanbuildinfo.cmake)
    include(${PROJECT_BINARY_DIR}/conan_paths.cmake)
endmacro()
