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
        .enableValidationLayers()
        .build(appInfo);
    (void)app;
}