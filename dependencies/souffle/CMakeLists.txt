cmake_minimum_required(VERSION 3.21)
project(InstallSouffle LANGUAGES C CXX)

include(ExternalProject)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

message(STATUS "Dependency \"souffle\"...")

# ---------------------------------------
# Build Souffle as an ExternalProject
# ---------------------------------------

list(APPEND CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    -DSOUFFLE_USE_ZLIB=OFF
    -DSOUFFLE_USE_SQLITE=OFF
    -DSOUFFLE_USE_OPENMP=OFF
    -DSOUFFLE_USE_CURSES=OFF
    -DSOUFFLE_USE_LIBFFI=OFF
)

message(STATUS "Preparing external project \"souffle\" with args:")
foreach(CMAKE_ARG ${CMAKE_ARGS})
    message(STATUS "-- ${CMAKE_ARG}")
endforeach()

ExternalProject_Add(
    souffle
    GIT_REPOSITORY https://github.com/drexlerd/souffle.git
    GIT_TAG ae5e9a1a7a133ffc58c9d9606ceb34b18960ba05
    PREFIX       ${CMAKE_BINARY_DIR}/souffle
    CMAKE_ARGS   ${CMAKE_ARGS}
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(souffle SOURCE_DIR BINARY_DIR)
message(STATUS "SOUFFLE SOURCE_DIR: ${SOURCE_DIR}")
message(STATUS "SOUFFLE BINARY_DIR: ${BINARY_DIR}")

# Build-tree artifacts
set(SOUFFLE_INC_BUILD            "${SOURCE_DIR}/src")
set(SOUFFLE_LIB_BUILD            "${BINARY_DIR}/src/libsouffle.a")

# --- Imported target for build-tree consumers --------------------------------
add_library(Souffle::souffle STATIC IMPORTED GLOBAL)
add_dependencies(Souffle::souffle souffle)
set_target_properties(Souffle::souffle PROPERTIES
  IMPORTED_LOCATION "${SOUFFLE_LIB_BUILD}"
  INTERFACE_INCLUDE_DIRECTORIES "${SOUFFLE_INC_BUILD}"
)

# --- Install headers + static lib --------------------------------------------
install(DIRECTORY "${SOUFFLE_INC_BUILD}/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/souffle"
        FILES_MATCHING PATTERN "*.h")

install(FILES "${SOUFFLE_LIB_BUILD}"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}")

# --- Export an INTERFACE target that carries include dirs in the install tree -
add_library(Souffle_export INTERFACE)
target_include_directories(Souffle_export INTERFACE
  "$<BUILD_INTERFACE:${SOUFFLE_INC_BUILD};${SOUFFLE_INC_BUILD}/souffle/include>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/souffle;${CMAKE_INSTALL_INCLUDEDIR}/souffle/include>"
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
