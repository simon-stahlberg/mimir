add_executable(mimir-profile "profiling.cpp")

find_package(benchmark "1.8.3" REQUIRED benchmark Main PATHS "${CMAKE_PREFIX_PATH}" NO_DEFAULT_PATH)
# Set result variables
find_package(benchmark)

target_link_libraries(mimir-profile PRIVATE mimir::core benchmark::benchmark)

set_property(TARGET mimir-profile PROPERTY CXX_STANDARD 17)
