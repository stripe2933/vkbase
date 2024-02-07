#include <vulkan/vulkan_hpp_macros.hpp>

import vkbase;

#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#endif

int main() {
#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
    VULKAN_HPP_DEFAULT_DISPATCHER.init();
#endif

    constexpr vk::ApplicationInfo appInfo {
        "vkbase Test", 0,
        nullptr, 0,
        vk::makeApiVersion(0, 1, 0, 0),
    };

    const vkbase::AppWithSwapchain appWithSwapchain
        = vkbase::AppWithSwapchainBuilder{
            .appBuilder = vkbase::AppBuilder{
                .instanceExtensions = {
                    "VK_KHR_surface",
                    "VK_EXT_headless_surface",
                },
            }
#if __APPLE__
            .enablePotability()
#endif
            .enableValidationLayers()
        }
        .build(appInfo, [](vk::Instance instance) {
            constexpr vk::HeadlessSurfaceCreateInfoEXT createInfo{};
            return instance.createHeadlessSurfaceEXT(createInfo);
        }, { 640U, 480U });
    (void)appWithSwapchain;
}