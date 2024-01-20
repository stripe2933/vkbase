//
// Created by gomkyung2 on 1/20/24.
//

module;

#include <algorithm>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <tuple>
#include <vector>

export module vkbase:builders;

export import vulkan_hpp;
export import :primitives;

export namespace vkbase {
    template <typename QueueFamilyIndices = DefaultQueueFamilyIndices, typename Queues = DefaultQueues, typename... DevicePNexts>
    struct AppBuilder {
        struct DefaultPhysicalDeviceRater {
            [[nodiscard]] std::uint32_t operator()(vk::PhysicalDevice physicalDevice) const;
        };

        vk::InstanceCreateFlags instanceCreateFlags{};
        std::vector<const char*> instanceLayers{};
        std::vector<const char*> instanceExtensions{};
        std::function<std::uint32_t(vk::PhysicalDevice)> physicalDeviceRater = DefaultPhysicalDeviceRater{};
        std::function<QueueFamilyIndices(vk::PhysicalDevice)> queueFamilyIndexGetter
            = [](vk::PhysicalDevice physicalDevice) { return QueueFamilyIndices { physicalDevice }; };
        vk::PhysicalDeviceFeatures physicalDeviceFeatures{};
        std::function<std::vector<std::uint32_t>(const QueueFamilyIndices&)> uniqueQueueFamilyIndexGetter
            = [](const QueueFamilyIndices &queueFamilyIndices) { return queueFamilyIndices.getUniqueIndices(); };
        std::vector<const char*> deviceExtensions{};
        std::tuple<DevicePNexts...> devicePNexts{};
        std::function<Queues(const QueueFamilyIndices&, vk::Device)> queueGetter
            = [](const QueueFamilyIndices &queueFamilyIndices, vk::Device device) {
                return Queues { device, queueFamilyIndices };
            };

        AppBuilder &enableValidationLayers();
        AppBuilder &enablePotability();

        [[nodiscard]] App<QueueFamilyIndices, Queues> build(const vk::ApplicationInfo &appInfo);
    };

    template <typename QueueFamilyIndices = DefaultQueueFamilyIndices, typename Queues = DefaultQueues, typename... DevicePNexts>
    struct AppWithSwapchainBuilder{
        AppBuilder<QueueFamilyIndices, Queues, DevicePNexts...> appBuilder{};
        vk::Format swapchainFormat = vk::Format::eB8G8R8A8Srgb;
        vk::ColorSpaceKHR swapchainColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
        vk::PresentModeKHR swapchainPresentMode = vk::PresentModeKHR::eFifo;

        [[nodiscard]] AppWithSwapchain<QueueFamilyIndices, Queues> build(
            const vk::ApplicationInfo &appInfo,
            std::function<vk::SurfaceKHR(vk::Instance)> surfaceFunc,
            vk::Extent2D extent);
    };
}

// --------------------
// Implementation.
// --------------------

#define FWD(...) std::forward<decltype(__VA_ARGS__)>(__VA_ARGS__)
#define APP_BUILDER_TEMPLATE_ABRV template <typename QueueFamilyIndices, typename Queues, typename... DevicePNexts>
#define APP_BUILDER_ABRV AppBuilder<QueueFamilyIndices, Queues, DevicePNexts...>
#define APP_WITH_SWAPCHAIN_BUILDER_ABRV AppWithSwapchainBuilder<QueueFamilyIndices, Queues, DevicePNexts...>

template <typename T, typename V, typename P = std::identity>
void present(std::vector<T> &container, V &&value, P &&proj = {}) {
    if (std::ranges::find(container, value, FWD(proj)) == container.end()) {
        container.emplace_back(FWD(value));
    }
}

struct SVConstructor {
    template <typename T>
    constexpr std::string_view operator()(T &&x) const noexcept{
        return { FWD(x) };
    }
};
inline constexpr SVConstructor svConstructor{};

namespace vkbase {
    APP_BUILDER_TEMPLATE_ABRV
    std::uint32_t APP_BUILDER_ABRV::DefaultPhysicalDeviceRater::operator()(
        vk::PhysicalDevice physicalDevice) const {
        // Check if given device supports the required queue families.
        try {
            const QueueFamilyIndices queueFamilyIndices { physicalDevice }; (void)queueFamilyIndices;
        }
        catch (const std::runtime_error&) {
            return 0;
        }

        std::uint32_t score = 0;

        // Rate physical device by its ability.
        const vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
        if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
            score += 1000;
        }
        score += properties.limits.maxImageDimension2D;

        return score;
    }

    APP_BUILDER_TEMPLATE_ABRV
    APP_BUILDER_ABRV & APP_BUILDER_ABRV::enableValidationLayers() {
        present(instanceLayers, "VK_LAYER_KHRONOS_validation", svConstructor);
        return *this;
    }

    APP_BUILDER_TEMPLATE_ABRV
    APP_BUILDER_ABRV & APP_BUILDER_ABRV::enablePotability() {
        instanceCreateFlags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
        present(instanceExtensions, "VK_KHR_portability_enumeration", svConstructor);
        present(instanceExtensions, "VK_KHR_get_physical_device_properties2", svConstructor);
        present(deviceExtensions, "VK_KHR_portability_subset", svConstructor);
        return *this;
    }

    APP_BUILDER_TEMPLATE_ABRV
    App<QueueFamilyIndices, Queues> APP_BUILDER_ABRV::build(const vk::ApplicationInfo &appInfo) {
        vk::raii::Context context{};

        const vk::InstanceCreateInfo instanceCreateInfo {
            instanceCreateFlags,
            &appInfo,
            instanceLayers,
            instanceExtensions,
        };
        vk::raii::Instance instance { context, instanceCreateInfo };

        std::vector physicalDevices = instance.enumeratePhysicalDevices();
        if (physicalDevices.empty()) {
            throw std::runtime_error { "No physical device found." };
        }
        vk::raii::PhysicalDevice physicalDevice = [&] {
            auto it = std::ranges::max_element(physicalDevices, {}, [this](const vk::raii::PhysicalDevice &physicalDevice) {
                return physicalDeviceRater(*physicalDevice);
            });
            if (physicalDeviceRater(**it) == 0) {
                throw std::runtime_error { "No physical device for required configuration." };
            }

            return *it;
        }();

        const QueueFamilyIndices queueFamilyIndices = queueFamilyIndexGetter(*physicalDevice);

        constexpr float queuePriority = 1.f;
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        std::ranges::transform(uniqueQueueFamilyIndexGetter(queueFamilyIndices), std::back_inserter(queueCreateInfos), [=](std::uint32_t queueFamilyIndex) {
            return vk::DeviceQueueCreateInfo {
                {},
                queueFamilyIndex,
                1,
                &queuePriority,
            };
        });
        const vk::StructureChain deviceCreateInfoChain{
            vk::DeviceCreateInfo{
                {},
                queueCreateInfos,
                {},
                deviceExtensions,
                &physicalDeviceFeatures,
            },
            get<DevicePNexts>(devicePNexts)...
        };
        vk::raii::Device device { physicalDevice, deviceCreateInfoChain.get() };

        const Queues queues = queueGetter(queueFamilyIndices, *device);

        return {
            std::move(context),
            std::move(instance),
            std::move(physicalDevice),
            queueFamilyIndices,
            std::move(device),
            queues
        };
    }

    APP_BUILDER_TEMPLATE_ABRV
    AppWithSwapchain<QueueFamilyIndices, Queues> APP_WITH_SWAPCHAIN_BUILDER_ABRV::build(
        const vk::ApplicationInfo &appInfo,
        std::function<vk::SurfaceKHR(vk::Instance)> surfaceFunc,
        vk::Extent2D extent) {
        present(appBuilder.deviceExtensions, "VK_KHR_swapchain", svConstructor);

        vk::raii::Context context{};

        // Create instance.
        const vk::InstanceCreateInfo instanceCreateInfo {
            appBuilder.instanceCreateFlags,
            &appInfo,
            appBuilder.instanceLayers,
            appBuilder.instanceExtensions,
        };
        vk::raii::Instance instance { context, instanceCreateInfo };

        // Create surface;
        vk::raii::SurfaceKHR surface { instance, surfaceFunc(*instance) };

        // Create physical device.
        std::vector physicalDevices = instance.enumeratePhysicalDevices();
        if (physicalDevices.empty()) {
            throw std::runtime_error { "No physical device found." };
        }
        vk::raii::PhysicalDevice physicalDevice = std::move(physicalDevices.front());

        // Find queue family indices.
        // 1. Requested by AppBuilder.
        const QueueFamilyIndices queueFamilyIndices = appBuilder.queueFamilyIndexGetter(*physicalDevice);
        // 2. Present queue family index.
        const std::uint32_t presentQueueFamilyIndex = [&] {
            const std::size_t queueFamiliesCount = physicalDevice.getQueueFamilyProperties().size();
            for (std::uint32_t index = 0; index < queueFamiliesCount; ++index) {
                if (physicalDevice.getSurfaceSupportKHR(index, *surface)) {
                    return index;
                }
            }

            throw std::runtime_error { "Present not supported" };
        }();

        // If presentQueueFamilyIndex not exist in queueFamilyIndices, add it.
        const std::vector uniqueQueueFamilyIndices = [&] {
            std::vector indices = appBuilder.uniqueQueueFamilyIndexGetter(queueFamilyIndices);
            if (std::ranges::find(indices, presentQueueFamilyIndex) == indices.end()) {
                indices.push_back(presentQueueFamilyIndex);
            }
            return std::move(indices);
        }();

        constexpr float queuePriority = 1.f;
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        queueCreateInfos.reserve(uniqueQueueFamilyIndices.size());
        std::ranges::transform(uniqueQueueFamilyIndices, std::back_inserter(queueCreateInfos), [=](std::uint32_t queueFamilyIndex) {
            return vk::DeviceQueueCreateInfo {
                {},
                queueFamilyIndex,
                1,
                &queuePriority,
            };
        });

        // Create device with queueCreateInfos, also chain pNexts.
        const vk::StructureChain deviceCreateInfoChain{
            vk::DeviceCreateInfo{
                {},
                queueCreateInfos,
                {},
                appBuilder.deviceExtensions,
            },
            get<DevicePNexts>(appBuilder.devicePNexts)...
        };
        vk::raii::Device device { physicalDevice, deviceCreateInfoChain.get() };

        // Get queues from device.
        const Queues queues = appBuilder.queueGetter(queueFamilyIndices, *device);
        const vk::Queue presentQueue = (*device).getQueue(presentQueueFamilyIndex, 0);

        // Create swapchain.
        const vk::SurfaceCapabilitiesKHR capabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
        const std::uint32_t imageCount = std::max(capabilities.minImageCount + 1, capabilities.maxImageCount);

        const vk::SwapchainCreateInfoKHR createInfo {
            {},
            *surface,
            imageCount,
            swapchainFormat,
            swapchainColorSpace,
            extent,
            1,
            vk::ImageUsageFlagBits::eColorAttachment,
            vk::SharingMode::eExclusive,
            {},
            capabilities.currentTransform,
            vk::CompositeAlphaFlagBitsKHR::eOpaque,
            swapchainPresentMode,
        };
        vk::raii::SwapchainKHR swapchain { device, createInfo };

        std::vector<std::pair<vk::Image, vk::raii::ImageView>> swapchainImageAndViews;
        swapchainImageAndViews.reserve(imageCount);
        std::ranges::transform(swapchain.getImages(), std::back_inserter(swapchainImageAndViews), [this, &device](vk::Image image) {
            const vk::ImageViewCreateInfo createInfo {
                {},
                image,
                vk::ImageViewType::e2D,
                swapchainFormat,
                {},
                { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
            };
            return std::pair { image, vk::raii::ImageView { device, createInfo } };
        });

        return {
            {
                std::move(context),
                std::move(instance),
                std::move(physicalDevice),
                queueFamilyIndices,
                std::move(device),
                queues
            },
            std::move(surface),
            presentQueueFamilyIndex,
            presentQueue,
            std::move(swapchain),
            swapchainFormat,
            swapchainColorSpace,
            extent,
            swapchainPresentMode,
            std::move(swapchainImageAndViews),
        };
    }
}
