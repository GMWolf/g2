//
// Created by felix on 13/04/2021.
//

#include <g2/gfx_instance.h>

#include "vk.h"
#include "validation.h"
#include "physical_device.h"

namespace g2::gfx
{
    struct Instance::Impl
    {
        vk::Instance vkInstance;
        vk::PhysicalDevice physicalDevice;
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

        auto [enumerateResult, layers] = vk::enumerateInstanceExtensionProperties();

        auto physicalDeviceResult = pickPhysicalDevice(instance);
        if (!physicalDeviceResult.has_value())
        {
            return;
        }

        pImpl->physicalDevice = physicalDeviceResult.value();

    }

    Instance::~Instance()
    {

        pImpl->vkInstance.destroy();
    }
}