cmake_minimum_required(VERSION 3.26 FATAL_ERROR)
project(vkbase)

if(CMAKE_VERSION VERSION_LESS "3.28.0")
    if(CMAKE_VERSION VERSION_LESS "3.27.0")
        set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "2182bf5c-ef0d-489a-91da-49dbc3090d2a")
    else()
        set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "aa1f7df0-828a-4fcd-9afc-2dc80491aca7")
    endif()
    set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)
else()
    cmake_policy(VERSION 3.28)
endif()

if (CMAKE_CXX_STANDARD LESS 20)
    message(FATAL_ERROR "vkbase requires C++20 or newer.")
endif()

option(VKBASE_BUILD_TESTS "Build vkbase tests.")

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
