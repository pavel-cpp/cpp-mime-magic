cmake_minimum_required(VERSION 3.23)
project(MimeMagic LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(CURRENT_DIR ${CMAKE_CURRENT_LIST_DIR})

#set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")

include_directories(
        ${CURRENT_DIR}
        ${CURRENT_DIR}/loader
        ${CURRENT_DIR}/nodes
)

add_library(
        MimeMagic_static
        STATIC
        loader/mime_loader.cpp
        nodes/basic_mime_node.cpp
        nodes/date_node/date_node.cpp
        nodes/string_node/string_node.cpp
        nodes/numeric_node/numeric_node.cpp
)

add_library(
        MimeMagic_shared
        SHARED
        loader/mime_loader.cpp
        nodes/basic_mime_node.cpp
        nodes/date_node/date_node.cpp
        nodes/string_node/string_node.cpp
        nodes/numeric_node/numeric_node.cpp
)

# Installing libraries and headers
install(TARGETS MimeMagic_static MimeMagic_shared
        EXPORT MimeMagicTargets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)

install(DIRECTORY loader/ DESTINATION include/loader FILES_MATCHING PATTERN "*.h")
install(DIRECTORY nodes/ DESTINATION include/nodes FILES_MATCHING PATTERN "*.h")
