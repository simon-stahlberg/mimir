# ----------------
# Target Profiling
# ----------------
if (BUILD_PROFILING)
    add_executable(mimir-profile bin/profiling.cpp ${MIMIR_SRC_FILES})
    target_link_libraries(mimir-profile PRIVATE project_options mimir)
    target_compile_options(mimir-profile PRIVATE -pg)
endif ()
