module;

#include <optional>
#include <stdexcept>
#include <vector>

module vkbase;

namespace vkbase {
    std::vector<std::uint32_t> DefaultQueueFamilyIndices::getUniqueIndices() const noexcept {
        if (graphics == compute) {
            return { graphics };
        }

        return { graphics, compute };
    }

    DefaultQueueFamilyIndices::DefaultQueueFamilyIndices(vk::PhysicalDevice physicalDevice) {
        std::optional<std::uint32_t> graphics, compute;
        for (std::uint32_t index = 0; vk::QueueFamilyProperties property : physicalDevice.getQueueFamilyProperties()) {
            if (property.queueFlags & vk::QueueFlagBits::eGraphics) {
                graphics = index;
            }
            if (property.queueFlags & vk::QueueFlagBits::eCompute) {
                compute = index;
            }

            if (graphics && compute) {
                this->graphics = *graphics;
                this->compute = *compute;
                return;
            }

            ++index;
        }

        throw std::runtime_error { "Failed to get required queue family indices from physical device" };
    }

    DefaultQueues::DefaultQueues(vk::Device device, DefaultQueueFamilyIndices queueFamilyIndices)
        : graphics { device.getQueue(queueFamilyIndices.graphics, 0) },
          compute { device.getQueue(queueFamilyIndices.compute, 0) } {

    }
}