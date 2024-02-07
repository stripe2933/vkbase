cmake_minimum_required(VERSION 3.26 FATAL_ERROR)
project(vkbase
    VERSION 0.1.0
    LANGUAGES CXX
    DESCRIPTION "Vulkan bootstrap with Vulkan-Hpp RAII objects, including swapchain creation support."
    HOMEPAGE_URL "https://github.com/stripe2933/vkbase"
)

if (CMAKE_VERSION VERSION_LESS "3.28.0")
    if(CMAKE_VERSION VERSION_LESS "3.27.0")
        set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "2182bf5c-ef0d-489a-91da-49dbc3090d2a")
    else()
        set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "aa1f7df0-828a-4fcd-9afc-2dc80491aca7")
    endif()
    set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)
else()
    cmake_policy(VERSION 3.28)
endif()

# --------------------
# CMake options.
# --------------------

option(VKBASE_BUILD_TESTS "Build vkbase tests.")

# --------------------
# External dependencies.
# --------------------

find_package(Vulkan REQUIRED)

# --------------------
# Setup modules for external dependencies.
# --------------------

file(DOWNLOAD https://raw.githubusercontent.com/stripe2933/Cpp20Module/main/Vulkan.cmake
    ${PROJECT_BINARY_DIR}/Vulkan.cmake
)
include(${PROJECT_BINARY_DIR}/Vulkan.cmake)

# --------------------
# Project libraries.
# --------------------

add_library(vkbase
    impl/primitives.cpp
)
target_sources(vkbase PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS ${PROJECT_SOURCE_DIR}/interface
    FILES
        ${PROJECT_SOURCE_DIR}/interface/vkbase.cppm
        ${PROJECT_SOURCE_DIR}/interface/primitives.cppm
        ${PROJECT_SOURCE_DIR}/interface/builders.cppm
)
target_link_libraries(vkbase PUBLIC VulkanHppModule)

# --------------------
# Tests.
# --------------------

if (VKBASE_BUILD_TESTS)
    add_subdirectory(test)
endif()
