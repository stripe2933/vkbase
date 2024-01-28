cmake_minimum_required(VERSION 3.26 FATAL_ERROR)
project(vkbase
    VERSION 0.1.0
    LANGUAGES CXX
    DESCRIPTION "Vulkan bootstrap with Vulkan-Hpp RAII objects, including swapchain creation support."
    HOMEPAGE_URL "https://github.com/stripe2933/vkbase"
)

# --------------------
# CMake options.
# --------------------

option(VKBASE_BUILD_TESTS "Build vkbase tests.")

# --------------------
# CMake scripts.
# --------------------

include(cmake/EnableCpp20Module.cmake)

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

    # TODO: For now, MSVC can't handle Vulkan named module with smart handles. When it fixed, remove below lines.
    # See https://github.com/KhronosGroup/Vulkan-Hpp/blob/main/README.md#c20-named-module.
    if (MSVC)
        target_compile_definitions(VulkanHppModule PUBLIC VULKAN_HPP_NO_SMART_HANDLE)
    endif()
endif()

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
