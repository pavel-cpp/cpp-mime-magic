cmake_minimum_required(VERSION 3.23)
project(mime_magic VERSION 1.0.1 LANGUAGES CXX)

add_subdirectory(src)

add_custom_target(copy_docs
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/docs ${CMAKE_BINARY_DIR}/docs
        COMMENT "Copying documentation"
)

add_custom_target(copy_examples
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/examples ${CMAKE_BINARY_DIR}/examples
        COMMENT "Copying examples"
)

add_dependencies(${PROJECT_NAME}_shared copy_docs)
add_dependencies(${PROJECT_NAME}_shared copy_examples)
add_dependencies(${PROJECT_NAME}_static copy_docs)
add_dependencies(${PROJECT_NAME}_static copy_examples)

# Создание и установка конфигурационных файлов
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
)

configure_file(src/mime_magicConfig.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        @ONLY
)

install(DIRECTORY ${CMAKE_BINARY_DIR}/docs DESTINATION ./)
install(DIRECTORY ${CMAKE_BINARY_DIR}/examples DESTINATION ./)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION lib/cmake/${PROJECT_NAME}
)

install(EXPORT ${PROJECT_NAME}Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION lib/cmake/${PROJECT_NAME}
)
