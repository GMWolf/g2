//
// Created by felix on 14/04/2021.
//

#include "device.h"

#include <algorithm>
#include <array>
#include <cstdint>

#include "swapchain.h"
#include "validation.h"

namespace g2::gfx {

static std::array<const char *, 1> deviceExtensions{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

static QueueFamilyIndices findQueueFamilies(
    const vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface) {
  QueueFamilyIndices indices{};

  auto queueFamilies = physicalDevice.getQueueFamilyProperties();

  for (int index = 0; index < queueFamilies.size(); index++) {
    const auto &queueFamily = queueFamilies[index];
    if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
      indices.graphics = index;
    }
    if (physicalDevice.getSurfaceSupportKHR(index, surface).value) {
      indices.present = index;
    }

    if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute) {
      indices.compute = index;
    }
    if (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) {
      indices.transfer = index;
    }
  }

  return indices;
}

static bool checkDeviceExtensionSupport(vk::PhysicalDevice device) {
  auto [result, extensions] = device.enumerateDeviceExtensionProperties();

  for (const auto &extension : deviceExtensions) {
    if (std::find_if(extensions.begin(), extensions.end(),
                     [=](vk::ExtensionProperties &ext) {
                       return strcmp(ext.extensionName, extension);
                     }) == extensions.end()) {
      return false;
    }
  }

  return true;
}

static bool isDeviceSuitable(vk::PhysicalDevice device,
                             vk::SurfaceKHR surface) {
  // vk::PhysicalDeviceProperties properties = device.getProperties();
  // vk::PhysicalDeviceFeatures features = device.getFeatures();

  QueueFamilyIndices indices = findQueueFamilies(device, surface);

  bool indicesComplete =
      indices.graphics.has_value() && indices.present.has_value();
  bool extensionsSupported = checkDeviceExtensionSupport(device);
  bool swapChainAdequate = false;
  if (extensionsSupported) {
    SwapChainSupportDetails swapChainSupport =
        querySwapChainSupport(device, surface);
    swapChainAdequate = checkSwapChainAdequate(swapChainSupport);
  }

  return indicesComplete && extensionsSupported && swapChainAdequate;
}

std::optional<vk::PhysicalDevice> pickPhysicalDevice(vk::Instance instance,
                                                     vk::SurfaceKHR surface) {
  auto [result, devices] = instance.enumeratePhysicalDevices();
  if (result != vk::Result::eSuccess) {
    return {};
  }

  if (devices.empty()) {
    return {};
  }

  for (auto &device : devices) {
    if (isDeviceSuitable(device, surface)) {
      return device;
    }
  }

  return {};
}

std::optional<std::pair<vk::Device, QueueFamilyIndices>> createDevice(
    vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface) {
  QueueFamilyIndices queueFamilies = findQueueFamilies(physicalDevice, surface);

  float queuePriority = 1.0f;

  std::array<uint32_t, 2> uniqueQueueFamilies{queueFamilies.graphics.value(),
                                              queueFamilies.present.value()};

  std::sort(uniqueQueueFamilies.begin(), uniqueQueueFamilies.end());
  auto end =
      std::unique(uniqueQueueFamilies.begin(), uniqueQueueFamilies.end());

  vk::DeviceQueueCreateInfo queueCreateInfos[2];

  uint32_t queueCount = 0;
  for (auto queueIndex : std::span{uniqueQueueFamilies.begin(), end}) {
    queueCreateInfos[queueCount] = vk::DeviceQueueCreateInfo{
        .queueFamilyIndex = queueIndex,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority,
    };
    queueCount++;
  }

  vk::PhysicalDeviceFeatures deviceFeatures{};

  auto validationLayers = getValidationLayerNames();

  vk::DeviceCreateInfo deviceCreateInfo{
      .queueCreateInfoCount = queueCount,
      .pQueueCreateInfos = queueCreateInfos,
      .enabledLayerCount = static_cast<uint32_t>(validationLayers.size()),
      .ppEnabledLayerNames = validationLayers.data(),
      .enabledExtensionCount = deviceExtensions.size(),
      .ppEnabledExtensionNames = deviceExtensions.data(),
      .pEnabledFeatures = &deviceFeatures,
  };

  auto [result, device] = physicalDevice.createDevice(deviceCreateInfo);

  if (result != vk::Result::eSuccess) {
    return {};
  }

  return std::make_pair(device, queueFamilies);
}

}  // namespace g2::gfx