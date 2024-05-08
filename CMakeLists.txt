cmake_minimum_required(VERSION 3.23)
project(mime_magic)

add_library(
        ${PROJECT_NAME}
        STATIC
        mime_node.cpp
)

add_executable(
        ${PROJECT_NAME}_test
        test/main.cpp
)

target_link_libraries(
        ${PROJECT_NAME}_test
        PUBLIC
        ${PROJECT_NAME}
)