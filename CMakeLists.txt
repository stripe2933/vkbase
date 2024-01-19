cmake_minimum_required(VERSION 3.28)
project(vkbase)

if (CMAKE_CXX_STANDARD LESS 20)
    message(FATAL_ERROR "vkbase requires C++20 or newer.")
endif()

option(VKBASE_BUILD_TESTS "Build vkbase tests." OFF)

# --------------------
# External dependencies.
# --------------------

find_package(Vulkan REQUIRED)

# --------------------
# Project libraries.
# --------------------

add_library(vkbase)
target_sources(vkbase PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS ${PROJECT_SOURCE_DIR}/src
    FILES
        ${PROJECT_SOURCE_DIR}/src/vkbase.cppm
        ${PROJECT_SOURCE_DIR}/src/vkbase.primitives.cppm
        ${PROJECT_SOURCE_DIR}/src/vkbase.builders.cppm

    FILE_SET CXX_MODULES
    BASE_DIRS ${Vulkan_INCLUDE_DIR}
    FILES ${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm
)
target_link_libraries(vkbase PUBLIC Vulkan::Vulkan)
target_compile_definitions(vkbase PUBLIC
    VULKAN_HPP_NO_SETTERS
    VULKAN_HPP_NO_SMART_HANDLE
    VULKAN_HPP_NO_TO_STRING
)

# --------------------
# Tests.
# --------------------

if (VKBASE_BUILD_TESTS)
    add_subdirectory(test)
endif()
