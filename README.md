# vkbase

![Windows (Visual Studio 2022)](https://github.com/stripe2933/vkbase/actions/workflows/windows.yml/badge.svg)
![macOS (Clang 17)](https://github.com/stripe2933/vkbase/actions/workflows/macos.yml/badge.svg)
![Linux (GCC 14)](https://github.com/stripe2933/vkbase/actions/workflows/linux.yml/badge.svg)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Start Vulkan easily with Vulkan-Hpp RAII objects. It helps to create Vulkan device and swapchain with minimal 
parameters. Written in C++20 with module support.

```c++
import vkbase;

int main() {
    constexpr vk::ApplicationInfo appInfo {
        "vkbase Test", 0,
        nullptr, 0,
        vk::makeApiVersion(0, 1, 2, 0), // Use Vulkan 1.2.
    };

    // Create app with DefaultQueueFamilyIndices/DefaultQueues.
    const vkbase::App app = vkbase::AppBuilder{}
#if __APPLE__
        .enablePotability() // Enable portability extension on macOS.
#endif
#ifndef NDEBUG
        .enableValidationLayers() // Enable validation layers on debug build.
#endif
        .build(appInfo);
        
    // Now you can use:
    // app.instance:                            vk::raii::Instance
    // app.physicalDevice:                      vk::raii::PhysicalDevice
    // app.queueFamilyIndices.graphics:         std::uint32_t
    // app.queueFamilyIndices.present:          std::uint32_t
    // app.device:                              vk::raii::Device
    // app.queues.graphics, app.queues.compute: vk::Queue
    
    // Everything is RAII, so you don't need to destroy them manually.
}
```

## Integration to your project

You need CMake 3.26 or newer for module support, and module compatible compiler. Supported compilers are:

| Compiler | Version                                              |
| --- |------------------------------------------------------|
| MSVC | MSVC 14.34 toolset (provided with VS 17.4) and newer |
| Clang | LLVM/Clang 17.0 and newer                            |
| GCC | GCC 14 (after the 2023-09-20 daily bump) and newer   |

About module support in CMake, see [here](https://gitlab.kitware.com/cmake/cmake/-/issues/18355).

### 1. Using CMake's FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(
    vkbase
    GIT_REPOSITORY https://github.com/stripe2933/vkbase.git
    GIT_TAG main
)
FetchContent_MakeAvailable(vkbase)

target_link_libraries(${PROJECT_NAME} PRIVATE vkbase)
```

### 2. Using CPM

```cmake
CPMAddPackage("gh:stripe2933/vkbase#main")
target_link_libraries(${PROJECT_NAME} PRIVATE vkbase)
```

About `CPMAddPackage` command, see [here](https://github.com/cpm-cmake/CPM.cmake).

## Examples

### 1. Create Vulkan device and swapchain with GLFW surface.

```c++
#include <vector>

// For now, GLFW's Vulkan feature only enabled when including Vulkan-related header.
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>

import vkbase;

int main() {
    constexpr vk::ApplicationInfo appInfo {
        "vkbase Test", 0,
        nullptr, 0,
        vk::makeApiVersion(0, 1, 2, 0),
    };

    // You must initialize GLFW before querying required instance extensions and creating window.
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *window = glfwCreateWindow(640, 480, "vkbase Test", nullptr, nullptr);

    // AppWithSwapchainBuilder requires AppBuilder, surface and swapchain info.
    const vkbase::AppWithSwapchain appWithSwapchain
        = vkbase::AppWithSwapchainBuilder{
            .appBuilder = vkbase::AppBuilder{
                // To integrate GLFW with Vulkan, you must pass required instance extensions to AppBuilder.
                .instanceExtensions = [] {
                    std::uint32_t glfwExtensionCount;
                    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
                    return std::vector(glfwExtensions, glfwExtensions + glfwExtensionCount);
                }(),
            }
#if __APPLE__
            .enablePotability()
#endif
#ifndef NDEBUG
            .enableValidationLayers()
#endif
            ,
            .swapchainFormat = vk::Format::eB8G8R8A8Srgb,               // Can omit this field (default: eR8G8B8A8Srgb).
            .swapchainColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear,   // Can omit this field (default: eSrgbNonlinear).
            .swapchainUsage = vk::ImageUsageFlagBits::eColorAttachment, // Can omit this field (default: eColorAttachment).
            .swapchainPresentMode = vk::PresentModeKHR::eFifo,          // Can omit this field (default: eFifo).
        }
        .build(appInfo, [window](vk::Instance instance) {
            // The 2nd argument of AppWithSwapchainBuilder::build is a function to create surface.
            // It accepts vk::Instance and should return vk::SurfaceKHR.
            vk::SurfaceKHR surface;
            glfwCreateWindowSurface(instance, window, nullptr, &reinterpret_cast<VkSurfaceKHR&>(surface));
            return surface;
        }, [window] {
            // The 3rd argument of AppWithSwapchainBuilder::build is swapchain extent.
            // Invoke this lambda function to get framebuffer size of the created GLFW window.
            if (vk::SurfaceKHR surface; glfwCreateWindowSurface(instance, window, nullptr,
                &reinterpret_cast<VkSurfaceKHR&>(surface)) == VK_SUCCESS) {
                return surface;
            }

            throw std::runtime_error { "GLFW window surface creation failed" };
        }());
        
    // Now you can use:
    // instance, physicalDevice, queueFamilyIndices, device, queues from App.
    // appWithSwapchain.presentQueueFamilyIndex: std::uint32_t
    // appWithSwapchain.presentQueue:            vk::Queue
    // appWithSwapchain.swapchain:               vk::raii::SwapchainKHR
    // appWithSwapchain.swapchainInfo:           vk::SwapchainCreateInfoKHR
    // appWithSwapchain.swapchainImageAndViews:  std::vector<std::pair<vk::Image, vk::raii::ImageView>>

    glfwDestroyWindow(window);
    glfwTerminate();
}
```

### 3. Use custom physical device rater and device extensions.

The below example code create Vulkan device with dynamic rendering feature and headless surface. For this, we should query the physical device
properties and features, and check whether the device supports dynamic rendering feature.

You can specify `physicalDeviceRater` field of `AppBuilder` at the initialization time. It is a function that accepts
`vk::PhysicalDeivce` and rate score for it. The physical device with the highest score is selected. Also, you can specify
that the physical device is inadequate by returning zero.

Also, you can specify `devicePNexts` field of `AppBuilder` to chaining the pNext of `vk::DeviceCreateInfo`.

```c++
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
}
```

### 4. Use custom queue configurations.

vkbase support Default queue configurations, which support a single graphics/command queue.

In [`vkbase.primitives.cppm`](src/vkbase.primitives.cppm):

```c++
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
```

Also, if you not manually specify `QueueFamilyIndices` and `Queues` template parameter in `AppBuilder`
(and also `AppWithSwapchainBuilder`), it uses default queue configurations.

In [`vkbase.builders.cppm`](src/vkbase.builders.cppm):

```c++
template <typename QueueFamilyIndices = DefaultQueueFamilyIndices, typename Queues = DefaultQueues, typename... DevicePNexts>
struct AppBuilder {
    ...
    
    std::function<QueueFamilyIndices(vk::PhysicalDevice)> queueFamilyIndexGetter
        = [](vk::PhysicalDevice physicalDevice) { return QueueFamilyIndices { physicalDevice }; };
        
    ...
    
    std::function<std::vector<std::uint32_t>(const QueueFamilyIndices&)> uniqueQueueFamilyIndexGetter
        = [](const QueueFamilyIndices &queueFamilyIndices) { return queueFamilyIndices.getUniqueIndices(); };
        
    ...
    
    std::function<Queues(const QueueFamilyIndices&, vk::Device)> queueGetter
        = [](const QueueFamilyIndices &queueFamilyIndices, vk::Device device) {
            return Queues { device, queueFamilyIndices };
        };

    ...
};
```

If you want to use custom queue configuration, you have to initialize `App` with the above 3 fields manually.

For example, the below code uses a single transfer queue.

```c++
#include <vector>

import vkbase;

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
#ifndef NDEBUG
        .enableValidationLayers()
#endif
        .build(appInfo);
}
```