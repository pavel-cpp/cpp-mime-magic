cmake_minimum_required(VERSION 3.23)
project(tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_PREFIX_PATH ../mime_magic/lib/cmake/mime_magic)

find_package(mime_magic REQUIRED)

add_custom_target(copy_tests
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/test-data ${CMAKE_BINARY_DIR}/test-data
        COMMENT "Copying tests"
)

add_executable(${PROJECT_NAME} tests.cpp)

add_dependencies(${PROJECT_NAME} copy_tests)

target_link_libraries(${PROJECT_NAME} mime_magic::mime_magic_static)