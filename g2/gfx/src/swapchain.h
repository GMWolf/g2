//
// Created by felix on 14/04/2021.
//

#ifndef G2_SWAPCHAIN_H
#define G2_SWAPCHAIN_H

#include <vector>

#include "device.h"
#include <vulkan/vulkan.h>
namespace g2::gfx {
struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct SwapChain {
  VkSwapchainKHR swapchain;
  std::vector<VkImage> images;
  std::vector<VkImageView> imageViews;
  VkFormat format;
  VkExtent2D extent;

  uint32_t aquireImage(VkDevice device, VkSemaphore imageAvailableSemaphore);

  inline operator bool() const { return swapchain; }
  void shutdown(VkDevice device);
};

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device,
                                              VkSurfaceKHR surface);

bool checkSwapChainAdequate(const SwapChainSupportDetails &swapChainSupport);

SwapChain createSwapChain(VkDevice device, VkPhysicalDevice physicalDevice,
                          VkSurfaceKHR surface, VkExtent2D windowExtent,
                          QueueFamilyIndices familyIndices);
}  // namespace g2::gfx

#endif  // G2_SWAPCHAIN_H
