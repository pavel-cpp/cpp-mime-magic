cmake_minimum_required(VERSION 3.23)
project(mime_magic)

add_library(
        ${PROJECT_NAME}
        STATIC
        mime_node.cpp
)

if(D_TEST)

    add_executable(
            ${PROJECT_NAME}_test
            main.cpp
    )

    target_link_libraries(
            ${PROJECT_NAME}_test
            PUBLIC
            ${PROJECT_NAME}
    )

endif ()