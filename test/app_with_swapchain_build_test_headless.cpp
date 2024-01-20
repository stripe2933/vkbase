import vkbase;

int main() {
    constexpr vk::ApplicationInfo appInfo {
        "vkbase Test", 0,
        nullptr, 0,
        vk::makeApiVersion(0, 1, 2, 0),
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
#ifndef NDEBUG
            .enableValidationLayers()
#endif
        }
        .build(appInfo, [](vk::Instance instance) {
            constexpr vk::HeadlessSurfaceCreateInfoEXT createInfo{};
            return instance.createHeadlessSurfaceEXT(createInfo);
        }, { 640U, 480U });
    (void)appWithSwapchain;
}