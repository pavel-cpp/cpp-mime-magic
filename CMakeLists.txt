cmake_minimum_required(VERSION 3.23)
project(mime_magic CXX)

SET(CMAKE_CXX_STANDARD 17)

include_directories(
        src
        src/loader
        srd/node
)

add_library(
        ${PROJECT_NAME}
        STATIC
        src/loader/mime_loader.cpp
        src/nodes/basic_mime_node.cpp
        src/nodes/date_node/date_node.cpp
        src/nodes/string_node/string_node.cpp
        src/loader/loading_error.h
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