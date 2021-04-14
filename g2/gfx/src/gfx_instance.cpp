//
// Created by felix on 13/04/2021.
//

#include <g2/gfx_instance.h>

#include "vk.h"
#include "validation.h"
#include "device.h"
#include "swapchain.h"
#include <glm/glm.hpp>

namespace g2::gfx
{
    struct Instance::Impl
    {
        vk::Instance vkInstance;
        vk::PhysicalDevice physicalDevice;
        vk::Device vkDevice;
        vk::Queue graphicsQueue;
        vk::Queue presentQueue;

        SwapChain swapChain;

        vk::SurfaceKHR surface;
    };

    Instance::Instance(const InstanceConfig& config)
    {
        pImpl = std::make_unique<Impl>();

        if (!checkValidationSupport())
        {
            return;
        }

         vk::ApplicationInfo appInfo
         {
            .pApplicationName = "unnamed g2 app",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "g2",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_2,
        };

        auto validationLayers = getValidationLayerNames();

        vk::InstanceCreateInfo createInfo
        {
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<uint32_t>(validationLayers.size()),
            .ppEnabledLayerNames = validationLayers.data(),
            .enabledExtensionCount = static_cast<uint32_t>(config.vkExtensions.size()),
            .ppEnabledExtensionNames = config.vkExtensions.data(),
        };

        auto [instanceResult, instance] = vk::createInstance(createInfo);

        if (instanceResult != vk::Result::eSuccess)
        {
            return;
        }

        pImpl->vkInstance = instance;

        pImpl->surface = config.application->createSurface(pImpl->vkInstance);

        auto physicalDeviceResult = pickPhysicalDevice(instance, pImpl->surface);
        if (!physicalDeviceResult)
        {
            return;
        }

        pImpl->physicalDevice = physicalDeviceResult.value();

        auto deviceResult = createDevice(pImpl->physicalDevice, pImpl->surface);
        if (!deviceResult)
        {
            return;
        }
        pImpl->vkDevice = deviceResult.value().first;
        QueueFamilyIndices queueFamilyIndices = deviceResult.value().second;

        pImpl->graphicsQueue = pImpl->vkDevice.getQueue(queueFamilyIndices.graphics.value(), 0);
        pImpl->presentQueue = pImpl->vkDevice.getQueue(queueFamilyIndices.present.value(), 0);

        glm::ivec2 appSize = config.application->getWindowSize();
        vk::Extent2D appExtent{static_cast<uint32_t>(appSize.x), static_cast<uint32_t>(appSize.y)};

        auto swapChain = createSwapChain(pImpl->vkDevice, pImpl->physicalDevice, pImpl->surface, appExtent, queueFamilyIndices);
        if (!swapChain.has_value())
        {
            return;
        }

        pImpl->swapChain = swapChain.value();
    }

    Instance::~Instance()
    {
        pImpl->vkDevice.destroy();
        pImpl->vkInstance.destroySurfaceKHR(pImpl->surface);
        pImpl->vkInstance.destroy();
    }
}