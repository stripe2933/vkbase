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
# Setup modules for external dependencies.
# --------------------

if (NOT TARGET VulkanHppModule)
    # Require Vulkan version ≥ 1.3.256 (earliest version when the Vulkan module was available)
    if (${Vulkan_VERSION} VERSION_LESS "1.3.256")
        message(FATAL_ERROR "Minimum required Vulkan version for C++ modules is 1.3.256. Found ${Vulkan_VERSION}.")
    endif()

    # Setup Vulkan C++ module as a library
    add_library(VulkanHppModule)
    target_sources(VulkanHppModule PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS ${Vulkan_INCLUDE_DIR}
        FILES ${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm
    )
    target_link_libraries(VulkanHppModule PUBLIC Vulkan::Vulkan)
endif()

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
)
target_link_libraries(vkbase PUBLIC VulkanHppModule)

# --------------------
# Tests.
# --------------------

if (VKBASE_BUILD_TESTS)
    add_subdirectory(test)
endif()
