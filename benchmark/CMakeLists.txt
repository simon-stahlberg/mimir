add_executable(profiling profiling.cpp ${MIMIR_SRC_FILES})
set_property(TARGET profiling PROPERTY CXX_STANDARD 17)
target_link_libraries(profiling mimir::core)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(profiling PRIVATE NDEBUG)
endif()

if(MSVC)
# Add MSVC specific library linking here
else()
    target_link_libraries(profiling -lstdc++fs)

    # These settings seem to cause issues with torch.
    # target_link_libraries(profiling -static-libstdc++ -static-libgcc)
endif()
