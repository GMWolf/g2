//
// Created by felix on 14/04/2021.
//

#ifndef G2_DEVICE_H
#define G2_DEVICE_H

#include <optional>

#include "vk.h"

namespace g2::gfx {

struct QueueFamilyIndices {
  std::optional<uint32_t> graphics{};
  std::optional<uint32_t> present{};
  std::optional<uint32_t> compute{};
  std::optional<uint32_t> transfer{};
};

std::optional<vk::PhysicalDevice> pickPhysicalDevice(vk::Instance instance,
                                                     vk::SurfaceKHR surface);

std::optional<std::pair<vk::Device, QueueFamilyIndices>> createDevice(
    vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);

}  // namespace g2::gfx

#endif  // G2_DEVICE_H
