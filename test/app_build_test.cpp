import vulkan_hpp;
import vkbase;

int main() {
    constexpr vk::ApplicationInfo appInfo {
        "vkbase Test", 0,
        nullptr, 0,
        vk::makeApiVersion(0, 1, 2, 0),
    };

    const vkbase::App app = vkbase::AppBuilder{}
#if __APPLE__
        .enablePotability()
#endif
#ifndef NDEBUG
        .enableValidationLayers()
#endif
        .build(appInfo);
    (void)app;
}