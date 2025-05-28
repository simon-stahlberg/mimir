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

    file(GLOB_RECURSE TMP_PUBLIC_HEADER_FILES "../include/mimir/${subdir}/*.hpp" "../include/mimir/${subdir}/**/*.hpp")
    list(APPEND MIMIR_PUBLIC_HEADER_FILES ${TMP_PUBLIC_HEADER_FILES})

    file(GLOB_RECURSE TMP_PUBLIC_HEADER_FILES "../include/cista/*.h" "../include/cista/**/*.h")
    list(APPEND MIMIR_PUBLIC_HEADER_FILES ${TMP_PUBLIC_HEADER_FILES})

    file(GLOB_RECURSE TMP_PUBLIC_HEADER_FILES "../include/nanobind/*.h" "../include/nanobind/**/*.h")
    list(APPEND MIMIR_PUBLIC_HEADER_FILES ${TMP_PUBLIC_HEADER_FILES})
endforeach()

list(APPEND MIMIR_PUBLIC_HEADER_FILES "../include/mimir/mimir.hpp")

add_library(core STATIC ${MIMIR_SRC_FILES} ${MIMIR_PRIVATE_HEADER_FILES} ${MIMIR_PUBLIC_HEADER_FILES})
set_target_properties(core PROPERTIES OUTPUT_NAME mimir_core)

# Create an alias for simpler reference
add_library(mimir::core ALIAS core)

target_link_libraries(core loki::parsers)
target_link_libraries(core Nauty::Nauty)
target_link_libraries(core fmt::fmt)
target_link_libraries(core -pthread)
target_link_libraries(core absl::flat_hash_set absl::flat_hash_map)
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
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
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
