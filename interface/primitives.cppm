//
// Created by gomkyung2 on 1/20/24.
//

module;

#include <algorithm>
#include <iterator>
#include <vector>

export module vkbase:primitives;

export import vulkan_hpp;

export namespace vkbase {
    struct DefaultQueueFamilyIndices {
        std::uint32_t graphics, compute;

        explicit DefaultQueueFamilyIndices(vk::PhysicalDevice physicalDevice);

        [[nodiscard]] std::vector<std::uint32_t> getUniqueIndices() const noexcept;
    };

    struct DefaultQueues {
        vk::Queue graphics;
        vk::Queue compute;

        DefaultQueues(vk::Device device, DefaultQueueFamilyIndices queueFamilyIndices);
    };

    template <typename QueueFamilyIndices = DefaultQueueFamilyIndices, typename Queues = DefaultQueues>
    struct App{
        vk::raii::Context context;
        vk::raii::Instance instance;
        vk::raii::PhysicalDevice physicalDevice;
        QueueFamilyIndices queueFamilyIndices;
        vk::raii::Device device;
        Queues queues;
    };

    template <typename QueueFamilyIndices = DefaultQueueFamilyIndices, typename Queues = DefaultQueues>
    struct AppWithSwapchain : App<QueueFamilyIndices, Queues>{
        vk::raii::SurfaceKHR surface;
        std::uint32_t presentQueueFamilyIndex;
        vk::Queue presentQueue;
        vk::raii::SwapchainKHR swapchain;
        vk::Format swapchainFormat;
        vk::ColorSpaceKHR swapchainColorSpace;
        vk::Extent2D swapchainExtent;
        vk::PresentModeKHR swapchainPresentMode;
        std::vector<std::pair<vk::Image, vk::raii::ImageView>> swapchainImageAndViews;

        void recreateSwapchain(vk::Format format, vk::ColorSpaceKHR colorSpace, vk::PresentModeKHR presentMode, vk::Extent2D extent);
    };
}

// --------------------
// Implementation.
// --------------------

namespace vkbase {
    template <typename QueueFamilyIndices, typename Queues>
    void AppWithSwapchain<QueueFamilyIndices, Queues>::recreateSwapchain(
        vk::Format format,
        vk::ColorSpaceKHR colorSpace,
        vk::PresentModeKHR presentMode,
        vk::Extent2D extent) {
        const vk::SurfaceCapabilitiesKHR capabilities
            = App<QueueFamilyIndices, Queues>::physicalDevice.getSurfaceCapabilitiesKHR(*surface);
        const std::uint32_t imageCount = swapchainImageAndViews.size();

        const vk::SwapchainCreateInfoKHR createInfo {
            {},
            *surface,
            imageCount,
            format,
            colorSpace,
            extent,
            1,
            vk::ImageUsageFlagBits::eColorAttachment,
            vk::SharingMode::eExclusive,
            {},
            capabilities.currentTransform,
            vk::CompositeAlphaFlagBitsKHR::eOpaque,
            presentMode,
            {},
            *swapchain,
        };
        swapchain = { { App<QueueFamilyIndices, Queues>::device, createInfo }, format, colorSpace, extent };

        swapchainFormat = format;
        swapchainColorSpace = colorSpace;
        swapchainExtent = extent;
        swapchainPresentMode = presentMode;

        swapchainImageAndViews.clear();
        swapchainImageAndViews.reserve(imageCount);
        std::ranges::transform(swapchain.getImages(), std::back_inserter(swapchainImageAndViews), [this](vk::Image image) {
            const vk::ImageViewCreateInfo createInfo {
                {},
                image,
                vk::ImageViewType::e2D,
                swapchainFormat,
                {},
                { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
            };
            return std::pair { image, vk::raii::ImageView { App<QueueFamilyIndices, Queues>::device, createInfo } };
        });
    }

}