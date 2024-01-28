include(CTest)

option(VKBASE_ENABLE_GLFW_INTEGRATION_TEST "Test Vulkan instance creation with GLFW surface.")

# --------------------
# External dependencies.
# --------------------

if (VKBASE_ENABLE_GLFW_INTEGRATION_TEST)
    find_package(GLFW3 CONFIG REQUIRED)
endif()

# --------------------
# Project executables.
# --------------------

add_executable(vkbase_app_build_test app_build_test.cpp)
target_link_libraries(vkbase_app_build_test PRIVATE vkbase)

add_executable(vkbase_app_with_swapchain_build_test_headless app_with_swapchain_build_test_headless.cpp)
target_link_libraries(vkbase_app_with_swapchain_build_test_headless PRIVATE vkbase)

if (VKBASE_ENABLE_GLFW_INTEGRATION_TEST)
    add_executable(vkbase_app_with_swapchain_build_test_glfw app_with_swapchain_build_test_glfw.cpp)
    target_link_libraries(vkbase_app_with_swapchain_build_test_glfw PRIVATE vkbase glfw)
endif()

add_executable(vkbase_app_build_test_dynamic_rendering app_build_test_dynamic_rendering.cpp)
target_link_libraries(vkbase_app_build_test_dynamic_rendering PRIVATE vkbase)

add_executable(vkbase_app_build_test_custom_queue app_build_test_custom_queue.cpp)
target_link_libraries(vkbase_app_build_test_custom_queue PRIVATE vkbase)

# --------------------
# Test configurations.
# --------------------

enable_testing()
add_test(vkbase_app_build_test vkbase_app_build_test)
add_test(vkbase_app_with_swapchain_build_test_headless vkbase_app_with_swapchain_build_test_headless)
if (VKBASE_ENABLE_GLFW_INTEGRATION_TEST)
    add_test(vkbase_app_with_swapchain_build_test_glfw vkbase_app_with_swapchain_build_test_glfw)
endif()
add_test(vkbase_app_build_test_dynamic_rendering vkbase_app_build_test_dynamic_rendering)
add_test(vkbase_app_build_test_custom_queue vkbase_app_build_test_custom_queue)