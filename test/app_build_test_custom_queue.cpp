#include <vector>

#include <vulkan/vulkan_hpp_macros.hpp>

import vkbase;

#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#endif

struct TransferQueueFamilyIndex {
    std::uint32_t transfer;

    explicit TransferQueueFamilyIndex(vk::PhysicalDevice physicalDevice) {
        for (std::uint32_t index = 0; vk::QueueFamilyProperties property : physicalDevice.getQueueFamilyProperties()) {
            if (property.queueFlags & vk::QueueFlagBits::eTransfer) {
                transfer = index;
                return;
            }

            ++index;
        }
    }
};

struct TransferQueue {
    vk::Queue transfer;
};

int main() {
#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
    VULKAN_HPP_DEFAULT_DISPATCHER.init();
#endif

    constexpr vk::ApplicationInfo appInfo {
        "vkbase Test", 0,
        nullptr, 0,
        vk::makeApiVersion(0, 1, 2, 0),
    };

    const vkbase::App app = vkbase::AppBuilder<TransferQueueFamilyIndex, TransferQueue>{
        // You don't need to specify queueFamilyIndexGetter, because it has same structure with DefaultQueueFamilyIndices
        // (constructor with vk::PhysicalDevice). But you can specify it if you want.
        // .queueFamilyIndexGetter
        //     = [](vk::PhysicalDevice physicalDevice) {
        //         return TransferQueueFamilyIndex { physicalDevice };
        //     },
        .uniqueQueueFamilyIndexGetter = [](const TransferQueueFamilyIndex &indices) {
            return std::vector { indices.transfer };
        },
        .queueGetter = [](const TransferQueueFamilyIndex &indices, vk::Device device) {
            return TransferQueue { device.getQueue(indices.transfer, 0) };
        },
    }
#if __APPLE__
        .enablePotability()
#endif
        .enableValidationLayers()
        .build(appInfo);
    (void)app;
}