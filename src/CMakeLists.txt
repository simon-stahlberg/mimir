file(GLOB_RECURSE MIMIR_SRC_FILES
   "*.cpp" "**/*.cpp")
file(GLOB_RECURSE MIMIR_PRIVATE_HEADER_FILES
    "*.hpp" "**/*.hpp")
file(GLOB_RECURSE MIMIR_PUBLIC_HEADER_FILES
    "../include/*.hpp" "../include/**/*.hpp")

add_library(core STATIC ${MIMIR_SRC_FILES} ${MIMIR_PRIVATE_HEADER_FILES} ${MIMIR_PUBLIC_HEADER_FILES})
set_target_properties(core PROPERTIES OUTPUT_NAME mimir_core)

# Create an alias for simpler reference
add_library(mimir::core ALIAS core)

# Apply the compile definition based on the selected representation
#if(STATE_REPR STREQUAL "Dense")
#    target_compile_definitions(core PUBLIC STATE_REPR_DENSE=1)
#endif()

target_link_libraries(core loki::parsers)
target_link_libraries(core flatmemory::flatmemory)
target_link_libraries(core Nauty::Nauty)

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
