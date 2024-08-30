find_package(GTest "1.11.0" REQUIRED COMPONENTS GTest Main PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
# Set result variables
find_package(GTest)

find_package(pybind11)

add_executable(pymimir_tests
        main_tests.cpp
        test_bindings.cpp
        )

target_link_libraries(pymimir_tests
        PRIVATE
        GTest::gtest
        pybind11::embed
        _pymimir_objects
        )

add_test(NAME pymimir_tests COMMAND pymimir_tests)