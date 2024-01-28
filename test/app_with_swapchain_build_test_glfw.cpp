#include <vector>

#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>

import vkbase;

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
            .enableValidationLayers()
        }
        .build(appInfo, [window](vk::Instance instance) {
            if (vk::SurfaceKHR surface; glfwCreateWindowSurface(instance, window, nullptr,
                &reinterpret_cast<VkSurfaceKHR&>(surface)) == VK_SUCCESS) {
                return surface;
            }
            throw std::runtime_error { "GLFW window surface creation failed" };
        }, [window] {
            int framebufferWidth, framebufferHeight;
            glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
            return vk::Extent2D { static_cast<std::uint32_t>(framebufferWidth), static_cast<std::uint32_t>(framebufferHeight) };
        }());
    (void)appWithSwapchain;

    glfwDestroyWindow(window);
    glfwTerminate();
}