cmake_minimum_required(VERSION 3.23)
project(mime_magic)

include_directories(
        src
        src/loader
        srd/node
)

add_library(
        ${PROJECT_NAME}
        STATIC
        src/node/mime_node.cpp
        src/loader/mime_loader.cpp
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