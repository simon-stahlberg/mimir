cmake_minimum_required(VERSION 3.21)
project(InstallClingo)

include(ExternalProject)

message(STATUS "Dependency \"clingo\"...")

# Platform-specific RPATH
if(APPLE)
    set(CLINGO_RPATH "@loader_path")
elseif(UNIX AND NOT APPLE)
    set(CLINGO_RPATH "$ORIGIN")
elseif(WIN32)
# Windows doesn't support RPATH, dynamic libs must be co-located
    set(CLINGO_RPATH "")
endif()

if(CLINGO_RPATH)
    list(APPEND CMAKE_ARGS
        -DCMAKE_INSTALL_RPATH=${CLINGO_RPATH}
    )
endif()

list(APPEND CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
    -DCLINGO_MANAGE_RPATH=OFF
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    -DCMAKE_POLICY_VERSION_MINIMUM=3.21  # override require cmake version
)

message(STATUS "Preparing external project \"clingo\" with args:")
foreach(CMAKE_ARG ${CMAKE_ARGS})
    message(STATUS "-- ${CMAKE_ARG}")
endforeach()
ExternalProject_Add(
    clingo
    GIT_REPOSITORY https://github.com/potassco/clingo.git
    GIT_TAG 65c976c0396405823ec65dd9eadb305511ba51eb
    PREFIX ${CMAKE_BINARY_DIR}/clingo
    GIT_SUBMODULES_RECURSE ON
    CMAKE_ARGS ${CMAKE_ARGS}
)
