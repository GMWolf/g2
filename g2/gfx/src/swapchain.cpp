//
// Created by felix on 14/04/2021.
//

#include "swapchain.h"

#include <vector>

#include "device.h"

namespace g2::gfx {
SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device,
                                              vk::SurfaceKHR surface) {
  SwapChainSupportDetails details;

  details.capabilities = device.getSurfaceCapabilitiesKHR(surface).value;
  details.formats = device.getSurfaceFormatsKHR(surface).value;
  details.presentModes = device.getSurfacePresentModesKHR(surface).value;

  return details;
}

bool checkSwapChainAdequate(const SwapChainSupportDetails &swapChainSupport) {
  return !swapChainSupport.formats.empty() &&
         !swapChainSupport.presentModes.empty();
}

static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
  for (const auto &format : availableFormats) {
    if (format.format == vk::Format::eR8G8B8A8Srgb &&
        format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      return format;
    }
  }
  return availableFormats[0];
}

static vk::PresentModeKHR choosePresentMode(
    const std::vector<vk::PresentModeKHR> &availablePresentModes) {
  for (const auto &presentMode : availablePresentModes) {
    if (presentMode == vk::PresentModeKHR::eMailbox) {
      return presentMode;
    }
  }

  return vk::PresentModeKHR::eFifo;
}

static vk::Extent2D chooseSwapExtent(
    const vk::SurfaceCapabilitiesKHR &capabilities, vk::Extent2D windowExtent) {
  if (capabilities.currentExtent.width != UINT32_MAX) {
    return capabilities.currentExtent;
  } else {
    vk::Extent2D extent{};
    extent.width =
        std::clamp(windowExtent.width, capabilities.minImageExtent.width,
                   capabilities.maxImageExtent.width);
    extent.height =
        std::clamp(windowExtent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);
    return extent;
  }
}

struct vkSwapChain {
  vk::SwapchainKHR swapchainKhr;
  vk::SurfaceFormatKHR surfaceFormat;
  vk::Extent2D extent2D;
};

static std::optional<vkSwapChain> createVKSwapChain(
    vk::Device device, vk::PhysicalDevice physicalDevice,
    vk::SurfaceKHR surface, vk::Extent2D windowExtent,
    QueueFamilyIndices familyIndices) {
  SwapChainSupportDetails swapChainSupport =
      querySwapChainSupport(physicalDevice, surface);

  vk::SurfaceFormatKHR surfaceFormat =
      chooseSwapSurfaceFormat(swapChainSupport.formats);
  vk::PresentModeKHR presentMode =
      choosePresentMode(swapChainSupport.presentModes);
  vk::Extent2D extent =
      chooseSwapExtent(swapChainSupport.capabilities, windowExtent);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 &&
      imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  vk::SwapchainCreateInfoKHR createInfo{
      .surface = surface,
      .minImageCount = imageCount,
      .imageFormat = surfaceFormat.format,
      .imageColorSpace = surfaceFormat.colorSpace,
      .imageExtent = extent,
      .imageArrayLayers = 1,
      .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
      .preTransform = swapChainSupport.capabilities.currentTransform,
      .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
      .presentMode = presentMode,
      .clipped = true,
      .oldSwapchain = {},
  };

  if (familyIndices.graphics != familyIndices.present) {
    uint32_t indices[] = {familyIndices.graphics.value(),
                          familyIndices.present.value()};
    createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = indices;
  } else {
    createInfo.imageSharingMode = vk::SharingMode::eExclusive;
  }

  auto result = device.createSwapchainKHR(createInfo);

  if (result.result != vk::Result::eSuccess) {
    return {};
  }

  return vkSwapChain{
      .swapchainKhr = result.value,
      .surfaceFormat = surfaceFormat,
      .extent2D = extent,
  };
}

static std::vector<vk::ImageView> createImageViews(
    vk::Device device, const std::vector<vk::Image> &images,
    vk::Format format) {
  std::vector<vk::ImageView> views(images.size());

  for (size_t i = 0; i < images.size(); i++) {
    vk::ComponentMapping components{
        .r = vk::ComponentSwizzle::eIdentity,
        .g = vk::ComponentSwizzle::eIdentity,
        .b = vk::ComponentSwizzle::eIdentity,
        .a = vk::ComponentSwizzle::eIdentity,
    };

    vk::ImageSubresourceRange subresourceRange{
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };

    vk::ImageViewCreateInfo createInfo{
        .image = images[i],
        .viewType = vk::ImageViewType::e2D,
        .format = format,
        .components = components,
        .subresourceRange = subresourceRange,
    };

    auto [result, imageView] = device.createImageView(createInfo);

    if (result != vk::Result::eSuccess) {
      return {};
    }

    views[i] = imageView;
  }

  return std::move(views);
}

SwapChain createSwapChain(vk::Device device, vk::PhysicalDevice physicalDevice,
                          vk::SurfaceKHR surface, vk::Extent2D windowExtent,
                          QueueFamilyIndices familyIndices) {
  auto vkSwapChain = createVKSwapChain(device, physicalDevice, surface,
                                       windowExtent, familyIndices);
  if (!vkSwapChain) {
    return {};
  }

  std::vector<vk::Image> images =
      device.getSwapchainImagesKHR(vkSwapChain.value().swapchainKhr).value;
  std::vector<vk::ImageView> imageViews =
      createImageViews(device, images, vkSwapChain->surfaceFormat.format);
  return SwapChain{
      .swapchain = vkSwapChain.value().swapchainKhr,
      .images = std::move(images),
      .imageViews = std::move(imageViews),
      .format = vkSwapChain.value().surfaceFormat.format,
      .extent = vkSwapChain.value().extent2D,
  };
}

void SwapChain::shutdown(vk::Device device) {
  for (auto &view : imageViews) {
    device.destroyImageView(view);
    view = vk::ImageView{};
  }

  device.destroySwapchainKHR(swapchain);
  swapchain = vk::SwapchainKHR{};
}
}  // namespace g2::gfx