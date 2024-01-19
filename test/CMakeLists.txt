include(CTest)

option(ENABLE_GLFW_INTEGRATION_TEST "Test Vulkan instance creation with GLFW surface." OFF)

# --------------------
# External dependencies.
# --------------------

if (ENABLE_GLFW_INTEGRATION_TEST)
    find_package(GLFW3 CONFIG REQUIRED)
endif()

# --------------------
# Project executables.
# --------------------

add_executable(vkbase_test main.cpp)
target_link_libraries(vkbase_test PRIVATE vkbase)

if (ENABLE_GLFW_INTEGRATION_TEST)
    target_compile_definitions(vkbase_test PRIVATE VKBASE_ENABLE_GLFW_INTEGRATION_TEST)
    target_link_libraries(vkbase_test PRIVATE glfw)
endif()

# --------------------
# Test configurations.
# --------------------

enable_testing()
add_test(vkbase_test vkbase_test)