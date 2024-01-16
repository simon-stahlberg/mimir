file(
    GLOB_RECURSE
    ALL_CXX_SOURCE_FILES
    ${PROJECT_DIR}/**.[ch]pp
    ${PROJECT_DIR}/**.h)
# Adding clang-format target if executable is found
find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
    add_custom_target(clang-format COMMAND ${CLANG_FORMAT} -i -style=file ${ALL_CXX_SOURCE_FILES})
endif()
