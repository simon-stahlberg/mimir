cmake_minimum_required(VERSION 3.21)
project(InstallSouffle LANGUAGES C CXX)

include(ExternalProject)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# ---------------------------------------
# Options (tweak as you like)
# ---------------------------------------

message(STATUS "Dependency \"souffle\"...")

# ---------------------------------------
# Build Soufflé as an ExternalProject
# ---------------------------------------

list(APPEND CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    -DSOUFFLE_USE_SQLITE=OFF
)

message(STATUS "Preparing external project \"souffle\" with args:")
foreach(CMAKE_ARG ${CMAKE_ARGS})
    message(STATUS "-- ${CMAKE_ARG}")
endforeach()

ExternalProject_Add(
    souffle_ep
    GIT_REPOSITORY https://github.com/souffle-lang/souffle.git
    GIT_TAG 2.5
    PREFIX       ${CMAKE_BINARY_DIR}/souffle
    CMAKE_ARGS   ${CMAKE_ARGS}
)

ExternalProject_Get_Property(souffle_ep SOURCE_DIR BINARY_DIR)
message(STATUS "SOUFFLE SOURCE_DIR: ${SOURCE_DIR}")
message(STATUS "SOUFFLE BINARY_DIR: ${BINARY_DIR}")

# Build-tree artifacts
set(SOUFFLE_INC_PUBLIC_BUILD     "${SOURCE_DIR}/src/include")
set(SOUFFLE_INC_INTERNAL_BUILD   "${SOURCE_DIR}/src")
set(SOUFFLE_INC_GENERATED_BUILD  "${BINARY_DIR}/src")
set(SOUFFLE_LIB_BUILD            "${BINARY_DIR}/src/libsouffle.a")

# --- Imported target for build-tree consumers --------------------------------
add_library(Souffle::souffle STATIC IMPORTED GLOBAL)
add_dependencies(Souffle::souffle souffle_ep)
set_target_properties(Souffle::souffle PROPERTIES
  IMPORTED_LOCATION "${SOUFFLE_LIB_BUILD}"
  INTERFACE_INCLUDE_DIRECTORIES
    "${SOUFFLE_INC_PUBLIC_BUILD};${SOUFFLE_INC_INTERNAL_BUILD};${SOUFFLE_INC_GENERATED_BUILD}"
)

# --- Install headers + static lib --------------------------------------------
install(DIRECTORY "${SOUFFLE_INC_PUBLIC_BUILD}/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        FILES_MATCHING PATTERN "*.h")

install(DIRECTORY "${SOUFFLE_INC_INTERNAL_BUILD}/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/souffle-internal/src"
        FILES_MATCHING PATTERN "*.h")

install(DIRECTORY "${SOUFFLE_INC_GENERATED_BUILD}/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/souffle-internal/src"
        FILES_MATCHING PATTERN "*.h")

install(FILES "${SOUFFLE_LIB_BUILD}"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}")

# --- Export an INTERFACE target that carries include dirs in the install tree -
add_library(Souffle_export INTERFACE)
target_include_directories(Souffle_export INTERFACE
  "$<BUILD_INTERFACE:${SOUFFLE_INC_PUBLIC_BUILD};${SOUFFLE_INC_INTERNAL_BUILD};${SOUFFLE_INC_GENERATED_BUILD}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR};${CMAKE_INSTALL_INCLUDEDIR}/souffle-internal/src>"
)

install(TARGETS Souffle_export
        EXPORT  SouffleTargets)

install(EXPORT SouffleTargets
        NAMESPACE Souffle::
        FILE      SouffleTargets.cmake
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/souffle")

# --- Config + Version files ---------------------------------------------------
set(SOUFFLE_PKG_VERSION "2.5")

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/SouffleConfigVersion.cmake"
  VERSION ${SOUFFLE_PKG_VERSION}
  COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/SouffleConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/SouffleConfig.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/souffle"
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/SouffleConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/SouffleConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/souffle")
