if(BUILD_PYMIMIR)


    find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
    find_package(pybind11 REQUIRED)
    pybind11_add_module(pymimir lib/pymimir.cpp ${MIMIR_SRC_FILES})

    set_target_properties(pymimir PROPERTIES PREFIX "")
    target_link_libraries(
            pymimir
            PRIVATE
            project_options
            project_warnings
            mimir
            #${PYTHON_LIBRARIES}
    )

#    if(CMAKE_BUILD_TYPE STREQUAL "Release")
#        target_compile_definitions(pymimir PRIVATE NDEBUG)
#    endif()

    if(MSVC)
        # Add MSVC specific library linking here
    else()
        target_link_libraries(pymimir PRIVATE -lstdc++fs)
        # These settings seem to cause issues with torch.
        # target_link_libraries(pymimir -static-libstdc++ -static-libgcc)
    endif()

    if(${PYTHON_VENV_EXE})
        set(_python_exe ${PYTHON_VENV_EXE})
    else ()
        set(_python_exe ${Python3_EXECUTABLE})
    endif ()
    # Generate pyi files, add generate_pyi_files dependent on pymimir
    add_custom_target(generate_pyi_files ALL
            COMMENT "Generating .pyi files"
            COMMAND ${_python_exe} -m pybind11_stubgen --output-dir ${CMAKE_CURRENT_BINARY_DIR} pymimir
    )

    add_dependencies(generate_pyi_files pymimir)
endif()
