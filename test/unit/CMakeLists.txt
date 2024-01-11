enable_testing()

find_package(GTest REQUIRED)

file(GLOB_RECURSE MIMIR_TEST_SOURCE_FILES
    "*.cpp" "**/*.cpp")
file(GLOB_RECURSE MIMIR_TEST_HEADER_FILES
    "*.hpp" "**/*.hpp")

set(TEST_NAME ${CMAKE_PROJECT_NAME}_tests)

add_executable(${TEST_NAME} ${MIMIR_TEST_SOURCE_FILES} ${MIMIR_TEST_HEADER_FILES})

target_link_libraries(${TEST_NAME}
    PRIVATE
        mimir
        GTest::GTest
        GTest::Main)

add_test(domain_gtests ${TEST_NAME})
