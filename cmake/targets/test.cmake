if(BUILD_TESTS)
    find_package(GTest REQUIRED)
    add_executable(mimir-test ${MIMIR_TEST_FILES} ${MIMIR_SRC_FILES})
    target_link_libraries(
            mimir-test
            PRIVATE
            project_options
            mimir
            GTest::gtest
#            pybind11::module
#            pybind11::embed
#            $<$<NOT:$<BOOL:USE_PYBIND11_FINDPYTHON>>:Python3::Module>
            pthread
    )
endif()
