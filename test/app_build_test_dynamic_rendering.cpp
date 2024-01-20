//
// Created by gomkyung2 on 1/21/24.
//

#include <tuple>

import vkbase;

int main() {
    constexpr vk::ApplicationInfo appInfo {
        "vkbase test", 0,
        nullptr, 0,
        vk::makeApiVersion(0, 1, 2, 0),
    };

    const vkbase::AppWithSwapchain appWithSwapchain = vkbase::AppWithSwapchainBuilder{
        .appBuilder = vkbase::AppBuilder {
            .instanceExtensions = {
                "VK_KHR_surface",
                "VK_EXT_headless_surface",
            },
            .physicalDeviceRater = [](vk::PhysicalDevice physicalDevice) -> std::uint32_t {
                const vk::StructureChain features2 = physicalDevice.getFeatures2<
                    vk::PhysicalDeviceFeatures2,
                    vk::PhysicalDeviceDynamicRenderingFeaturesKHR>();
                if (!features2.get<vk::PhysicalDeviceDynamicRenderingFeaturesKHR>().dynamicRendering) {
                    return 0;
                }

                return vkbase::AppBuilder<>::DefaultPhysicalDeviceRater{}(physicalDevice);
            },
            .deviceExtensions = {
                // Dynamic rendering
                "VK_KHR_multiview",             // -> VK_KHR_create_renderpass2
                "VK_KHR_maintenance2",          // -> VK_KHR_create_renderpass2
                "VK_KHR_create_renderpass2",    // -> VK_KHR_depth_stencil_resolve
                "VK_KHR_depth_stencil_resolve", // -> VK_KHR_dynamic_rendering
                "VK_KHR_dynamic_rendering",
            },
            .devicePNexts = std::tuple {
                vk::PhysicalDeviceDynamicRenderingFeatures { vk::True },
            },
        }
#ifndef NDEBUG
        .enableValidationLayers()
#endif
#if __APPLE__
        .enablePotability()
#endif
    }
    .build(appInfo, [](vk::Instance instance) {
        constexpr vk::HeadlessSurfaceCreateInfoEXT createInfo{};
        return instance.createHeadlessSurfaceEXT(createInfo);
    }, { 640U, 480U });
    (void)appWithSwapchain;
}