#include <vector>

import vkbase;
// A trick for import GLFW Vulkan bindings without load huge vulkan headers.
// Note that this includes must be after vulkan_hpp.
// TODO: remove this when GLFW support Vulkan loading with module.
#define VK_VERSION_1_0
using VkInstance = vk::Instance::CType;
using VkSurfaceKHR = vk::SurfaceKHR::CType;
using VkPhysicalDevice = vk::PhysicalDevice::CType;
using VkAllocationCallbacks = vk::AllocationCallbacks;
using VkResult = vk::Result;
#include <GLFW/glfw3.h>

int main() {
    constexpr vk::ApplicationInfo appInfo {
        "vkbase Test", 0,
        nullptr, 0,
        vk::makeApiVersion(0, 1, 2, 0),
    };

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *window = glfwCreateWindow(640, 480, "vkbase Test", nullptr, nullptr);

    const vkbase::AppWithSwapchain appWithSwapchain
        = vkbase::AppWithSwapchainBuilder{
            .appBuilder = vkbase::AppBuilder{
                .instanceExtensions = [] {
                    std::uint32_t glfwExtensionCount;
                    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
                    return std::vector(glfwExtensions, glfwExtensions + glfwExtensionCount);
                }(),
            }
#if __APPLE__
            .enablePotability()
#endif
#ifndef NDEBUG
            .enableValidationLayers()
#endif
        }
        .build(appInfo, [window](vk::Instance instance) {
            vk::SurfaceKHR surface;
            glfwCreateWindowSurface(instance, window, nullptr, &reinterpret_cast<VkSurfaceKHR&>(surface));
            return surface;
        }, [window] {
            int framebufferWidth, framebufferHeight;
            glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
            return vk::Extent2D { static_cast<std::uint32_t>(framebufferWidth), static_cast<std::uint32_t>(framebufferHeight) };
        }());
    (void)appWithSwapchain;

    glfwDestroyWindow(window);
    glfwTerminate();
}