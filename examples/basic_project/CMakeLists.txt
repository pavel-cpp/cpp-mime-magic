cmake_minimum_required(VERSION 3.23)
project(test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_PREFIX_PATH ..\\mime_magic\\lib\\cmake\\MimeMagic)

find_package(MimeMagic REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} MimeMagic::MimeMagic_static)