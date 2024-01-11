file(GLOB_RECURSE MIMIR_SRC_FILES
   "*.cpp" "**/*.cpp")
file(GLOB_RECURSE MIMIR_PRIVATE_HEADER_FILES
    "*.hpp" "**/*.hpp")
file(GLOB_RECURSE MIMIR_PUBLIC_HEADER_FILES
    "../include/*.hpp" "../include/**/*.hpp")

add_library(mimir STATIC "${MIMIR_SRC_FILES}" "${MIMIR_PRIVATE_HEADER_FILES}" "${MIMIR_PUBLIC_HEADER_FILES}")

set_property(TARGET mimir PROPERTY CXX_STANDARD 17)

find_package(loki 0.0.1 COMPONENTS parsers REQUIRED)
target_link_libraries(mimir loki::parsers)

# Use include depending on building or using from installed location
target_include_directories(mimir
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

# Install the target and create export-set
install(
    TARGETS mimir
    EXPORT mimirTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Generate and install export file
install(EXPORT mimirTargets
    COMPONENT mimir
    FILE mimirTargets.cmake
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/mimir"
)

# Generate build tree export file
export(EXPORT mimirTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/mimirTargets.cmake"
)
