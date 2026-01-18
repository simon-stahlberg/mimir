option(MIMIR_BUILD_SHARED_CORE "Build the core Mimir library as shared" ON)

# List of subdirectory names
set(SUBDIRECTORIES "algorithms" "buffering" "common" "formalism" "search" "datasets" "graphs" "languages")

# Initialize empty lists for source and header files
set(MIMIR_SRC_FILES "")
set(MIMIR_PRIVATE_HEADER_FILES "")
set(MIMIR_PUBLIC_HEADER_FILES "")

# Loop over the subdirectories for source files
foreach(subdir ${SUBDIRECTORIES})
    file(GLOB_RECURSE TMP_SRC_FILES "${subdir}/*.cpp" "${subdir}/**/*.cpp")
    list(APPEND MIMIR_SRC_FILES ${TMP_SRC_FILES})

    file(GLOB_RECURSE TMP_PRIVATE_HEADER_FILES "${subdir}/*.hpp" "${subdir}/**/*.hpp")
    list(APPEND MIMIR_PRIVATE_HEADER_FILES ${TMP_PRIVATE_HEADER_FILES})

    file(GLOB_RECURSE TMP_PUBLIC_HEADER_FILES 
        "../include/mimir/${subdir}/*.hpp" "../include/mimir/${subdir}/**/*.hpp")
    list(APPEND MIMIR_PUBLIC_HEADER_FILES ${TMP_PUBLIC_HEADER_FILES})
endforeach()

file(GLOB_RECURSE TMP_PUBLIC_HEADER_FILES "../include/cista/*.h" "../include/cista/**/*.h")
list(APPEND MIMIR_PUBLIC_HEADER_FILES ${TMP_PUBLIC_HEADER_FILES})

file(GLOB_RECURSE TMP_PUBLIC_HEADER_FILES "../include/valla/*.hpp" "../include/valla/**/*.hpp")
list(APPEND MIMIR_PUBLIC_HEADER_FILES ${TMP_PUBLIC_HEADER_FILES})

list(APPEND MIMIR_PUBLIC_HEADER_FILES "../include/mimir/mimir.hpp")

if(MIMIR_BUILD_SHARED_CORE)
    add_library(core SHARED ${MIMIR_SRC_FILES} ${MIMIR_PRIVATE_HEADER_FILES} ${MIMIR_PUBLIC_HEADER_FILES})
else()
    add_library(core STATIC ${MIMIR_SRC_FILES} ${MIMIR_PRIVATE_HEADER_FILES} ${MIMIR_PUBLIC_HEADER_FILES})
endif()
set_target_properties(core PROPERTIES
    OUTPUT_NAME mimir_core
    POSITION_INDEPENDENT_CODE ON
)
# Convenience for Windows consumers; a proper export macro is preferable long-term.
if(WIN32 AND MIMIR_BUILD_SHARED_CORE)
    set_target_properties(core PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# Create an alias for simpler reference
add_library(mimir::core ALIAS core)

target_link_libraries(core
  PUBLIC
    valla::core
    loki::parsers
    absl::flat_hash_set
    absl::flat_hash_map
    TBB::tbb
  PRIVATE
    Threads::Threads
    Nauty::Nauty
    fmt::fmt
)

# target_include_directories(core PUBLIC ${Python3_INCLUDE_DIRS})
# target_link_libraries(core ${PYTHON3_LIB_DIRS})

# Use include depending on building or using from installed location
target_include_directories(core
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

# Install the target and create export-set
install(
    TARGETS core
    EXPORT mimircoreTargets
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

# Generate and install export file
install(EXPORT mimircoreTargets
    NAMESPACE mimir::
    COMPONENT core
    FILE "mimircoreTargets.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/mimir"
)

# Generate build tree export file
export(EXPORT mimircoreTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/mimircoreTargets.cmake"
    NAMESPACE mimir::
)
