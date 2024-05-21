cmake_minimum_required(VERSION 3.23)
project(MimeMagic VERSION 1.0.0 LANGUAGES CXX)

add_subdirectory(src)

add_custom_target(copy_docs
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/docs ${CMAKE_BINARY_DIR}/docs
        COMMENT "Copying documentation"
)

add_custom_target(copy_examples
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/examples ${CMAKE_BINARY_DIR}/examples
        COMMENT "Copying documentation"
)

add_dependencies(MimeMagic_shared copy_docs)
add_dependencies(MimeMagic_shared copy_examples)

# Создание и установка конфигурационных файлов
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/MimeMagicConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
)

configure_file(src/MimeMagicConfig.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/MimeMagicConfig.cmake"
        @ONLY
)

install(DIRECTORY ${CMAKE_BINARY_DIR}/docs DESTINATION ./)
install(DIRECTORY ${CMAKE_BINARY_DIR}/examples DESTINATION ./)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/MimeMagicConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/MimeMagicConfigVersion.cmake"
        DESTINATION lib/cmake/MimeMagic
)

install(EXPORT MimeMagicTargets
        FILE MimeMagicTargets.cmake
        NAMESPACE MimeMagic::
        DESTINATION lib/cmake/MimeMagic
)
