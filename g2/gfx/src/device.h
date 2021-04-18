//
// Created by felix on 14/04/2021.
//

#ifndef G2_DEVICE_H
#define G2_DEVICE_H

#include <optional>

#include "volk.h"

namespace g2::gfx {

struct QueueFamilyIndices {
  std::optional<uint32_t> graphics{};
  std::optional<uint32_t> present{};
  std::optional<uint32_t> compute{};
  std::optional<uint32_t> transfer{};
};

std::optional<VkPhysicalDevice> pickPhysicalDevice(VkInstance instance,
                                                     VkSurfaceKHR surface);

std::optional<std::pair<VkDevice, QueueFamilyIndices>> createDevice(
    VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

}  // namespace g2::gfx

#endif  // G2_DEVICE_H
