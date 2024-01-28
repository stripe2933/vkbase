//
// Created by gomkyung2 on 1/20/24.
//

module;

#include <algorithm>
#include <iterator>
#include <optional>
#include <span>
#include <stdexcept>
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
        vk::SwapchainCreateInfoKHR swapchainInfo;
        std::vector<std::pair<vk::Image, vk::raii::ImageView>> swapchainImageAndViews;

        [[nodiscard]] auto acquireSwapchainImageIndex(vk::Semaphore imageAvailableSema) const -> std::optional<std::uint32_t>;
        [[nodiscard]] auto presentSwapchainImage(std::uint32_t imageIndex, std::span<const vk::Semaphore> waitSemas = {}) const -> bool;
        void recreateSwapchain(vk::Extent2D extent);
    };
}

// --------------------
// Implementation.
// --------------------

namespace vkbase {
    template <typename QueueFamilyIndices, typename Queues>
    void AppWithSwapchain<QueueFamilyIndices, Queues>::recreateSwapchain(vk::Extent2D extent) {
        swapchainInfo.imageExtent = extent;
        swapchain = { App<QueueFamilyIndices, Queues>::device, swapchainInfo };
        std::ranges::transform(swapchain.getImages(), swapchainImageAndViews.begin(), [this](vk::Image image) {
            return std::pair { image, vk::raii::ImageView { App<QueueFamilyIndices, Queues>::device, {
                {},
                image,
                vk::ImageViewType::e2D,
                swapchainInfo.imageFormat,
                {},
                { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
            } } };
        });
    }

    template <typename QueueFamilyIndices, typename Queues>
    auto AppWithSwapchain<QueueFamilyIndices, Queues>::acquireSwapchainImageIndex(vk::Semaphore imageAvailableSema) const -> std::optional<std::uint32_t> {
        const auto [result, imageIndex] = (*App<QueueFamilyIndices, Queues>::device).acquireNextImageKHR(
            *swapchain, UINT64_MAX, imageAvailableSema);
        switch (result) {
            case vk::Result::eSuccess: case vk::Result::eSuboptimalKHR:
              return imageIndex;
            case vk::Result::eErrorOutOfDateKHR:
                return std::nullopt;
            default:
                throw std::runtime_error { "Acquiring swapchain image failed" };
        }
    }

    template <typename QueueFamilyIndices, typename Queues>
    auto AppWithSwapchain<QueueFamilyIndices, Queues>::presentSwapchainImage(std::uint32_t imageIndex, std::span<const vk::Semaphore> waitSemas) const -> bool{
        switch (presentQueue.presentKHR({ waitSemas, *swapchain, imageIndex })) {
            case vk::Result::eSuccess: return true;
            case vk::Result::eErrorOutOfDateKHR: case vk::Result::eSuboptimalKHR: return false;
            default: throw std::runtime_error { "Presenting swapchain image failed" };
        }
    }
}