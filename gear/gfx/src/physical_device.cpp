//
// Created by felix on 14/04/2021.
//

#include "physical_device.h"
#include <cstdint>

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphics {};
    std::optional<uint32_t> compute {};
    std::optional<uint32_t> transfer {};
};

static QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice& physicalDevice)
{
    QueueFamilyIndices indices {};

    auto queueFamilies = physicalDevice.getQueueFamilyProperties();

    for(int index = 0; index < queueFamilies.size(); index++)
    {
        const auto& queueFamily = queueFamilies[index];
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            indices.graphics = index;
        }
        else if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute)
        {
            indices.compute = index;
        }
        else if (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer)
        {
            indices.transfer = index;
        }
    }

    return indices;
}

static bool isDeviceSuitable(const vk::PhysicalDevice& device)
{
    //vk::PhysicalDeviceProperties properties = device.getProperties();
    //vk::PhysicalDeviceFeatures features = device.getFeatures();

    QueueFamilyIndices indices = findQueueFamilies(device);

    return indices.graphics.has_value();
}

std::optional<vk::PhysicalDevice> g2::gfx::pickPhysicalDevice(vk::Instance &instance) {
    auto[result, devices] = instance.enumeratePhysicalDevices();
    if (result != vk::Result::eSuccess)
    {
        return {};
    }

    if (devices.empty())
    {
        return {};
    }

    for(auto& device : devices)
    {
        if (isDeviceSuitable(device))
        {
            return device;
        }
    }

    return {};
}
