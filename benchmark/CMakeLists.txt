add_executable(mimir-profile "profiling.cpp")

find_package(benchmark REQUIRED)
target_link_libraries(mimir-profile PRIVATE mimir benchmark::benchmark)

set_property(TARGET mimir-profile PROPERTY CXX_STANDARD 17)
