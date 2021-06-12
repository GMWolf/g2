//
// Created by felix on 14/04/2021.
//

#include "device.h"

#include <algorithm>
#include <array>
#include <cstdint>

#include "swapchain.h"
#include "validation.h"
#include <cstring>

namespace g2::gfx {

static std::array<const char *, 1> deviceExtensions{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

static QueueFamilyIndices findQueueFamilies(
    const VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
  QueueFamilyIndices indices{};


  uint32_t queueFamilyPropsCount;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropsCount, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyPropsCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropsCount, queueFamilies.data());

  for (int index = 0; index < queueFamilies.size(); index++) {
    const auto &queueFamily = queueFamilies[index];
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphics = index;
    }

    VkBool32 surfaceSupported;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, index, surface, &surfaceSupported);
    if(surfaceSupported) {
      indices.present = index;
    }

    if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
      indices.compute = index;
    }
    if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
      indices.transfer = index;
    }
  }

  return indices;
}

static bool checkDeviceExtensionSupport(VkPhysicalDevice device) {

  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> extensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

  for (const auto &extension : deviceExtensions) {
    if (std::find_if(extensions.begin(), extensions.end(),
                     [=](const VkExtensionProperties &ext) {
                       return strcmp(ext.extensionName, extension);
                     }) == extensions.end()) {
      return false;
    }
  }

  return true;
}

static bool isDeviceSuitable(VkPhysicalDevice device,
                             VkSurfaceKHR surface) {
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

std::optional<VkPhysicalDevice> pickPhysicalDevice(VkInstance instance,
                                                     VkSurfaceKHR surface) {

  uint32_t physicalDeviceCount;
  vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

  if (physicalDeviceCount == 0){
    return {};
  }

  std::vector<VkPhysicalDevice> devices(physicalDeviceCount);
  if(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, devices.data()) != VK_SUCCESS){
    return {};
  }

  //return devices[1];

  for (auto &device : devices) {
    if (isDeviceSuitable(device, surface)) {
      return device;
    }
  }

  return {};
}

std::optional<std::pair<VkDevice, QueueFamilyIndices>> createDevice(
    VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
  QueueFamilyIndices queueFamilies = findQueueFamilies(physicalDevice, surface);

  float queuePriority = 1.0f;

  std::array<uint32_t, 2> uniqueQueueFamilies{queueFamilies.graphics.value(),
                                              queueFamilies.present.value()};

  std::sort(uniqueQueueFamilies.begin(), uniqueQueueFamilies.end());
  auto end =
      std::unique(uniqueQueueFamilies.begin(), uniqueQueueFamilies.end());

  VkDeviceQueueCreateInfo queueCreateInfos[2];

  uint32_t queueCount = 0;
  for (auto queueIndex : std::span{uniqueQueueFamilies.begin(), end}) {
    queueCreateInfos[queueCount] = VkDeviceQueueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queueIndex,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority,
    };
    queueCount++;
  }

  VkPhysicalDeviceFeatures deviceFeatures{};
  deviceFeatures.geometryShader = true;

  auto validationLayers = getValidationLayerNames();

  VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES
  };
  descriptorIndexingFeatures.descriptorBindingPartiallyBound = true;
  descriptorIndexingFeatures.runtimeDescriptorArray = true;

  VkDeviceCreateInfo deviceCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = &descriptorIndexingFeatures,
      .queueCreateInfoCount = queueCount,
      .pQueueCreateInfos = queueCreateInfos,
      .enabledLayerCount = static_cast<uint32_t>(validationLayers.size()),
      .ppEnabledLayerNames = validationLayers.data(),
      .enabledExtensionCount = deviceExtensions.size(),
      .ppEnabledExtensionNames = deviceExtensions.data(),
      .pEnabledFeatures = &deviceFeatures,
  };


  VkDevice device;
  if(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS) {
    return {};
  }

  return std::make_pair(device, queueFamilies);
}

}  // namespace g2::gfx